use std::env;
use std::fs::File;
use std::io::{self, BufRead, BufReader, Read, Write};
use std::path::{Path, PathBuf};
use std::time::Instant;

use anyhow::{Result, anyhow};
use configparser::ini::Ini;
use encoding_rs::WINDOWS_1251;
use encoding_rs_io::DecodeReaderBytesBuilder;
use quick_xml::Reader;
use quick_xml::events::Event;
use quick_xml::name::QName;

fn print_elapsed_time(text: &str, start: Instant, end: Instant) {
    println!(
        "# {}: {:.9} сек.",
        text,
        end.duration_since(start).as_secs_f64()
    );
}

fn config_items(config: &Ini, section: &str) -> Vec<(String, String)> {
    if let Some(sections_map) = config.get_map() {
        if let Some(section_map) = sections_map.get(section) {
            return section_map
                .iter()
                .filter_map(|(k, v)| v.as_ref().map(|val| (k.clone(), val.clone())))
                .collect();
        }
    }
    vec![]
}

fn main_process(appdir: &Path, workdir: &Path) -> Result<()> {
    let xml_path = get_xml_file()?;
    let start = Instant::now();

    let config = get_config(appdir, workdir)?;
    let end_get_config = Instant::now();

    let result = process_xml(&config, &xml_path)?;
    let end_process_xml = Instant::now();

    write_results(&result, workdir)?;
    let end = Instant::now();

    print_elapsed_time("* get config", start, end_get_config);
    print_elapsed_time("* process xml", end_get_config, end_process_xml);
    print_elapsed_time("* process results", end_process_xml, end);
    print_elapsed_time("Обработка завершена", start, end);
    Ok(())
}

fn get_xml_file() -> Result<PathBuf> {
    let args: Vec<String> = env::args().collect();
    let file = if args.len() > 1 {
        args[1].clone()
    } else {
        print!("# XML файл: ");
        io::stdout().flush()?;
        let mut input = String::new();
        io::stdin().read_line(&mut input)?;
        input.trim().to_string()
    };

    let path = PathBuf::from(file);
    if path.exists() {
        Ok(path.canonicalize()?)
    } else {
        Err(anyhow!("# {} не найден", path.display()))
    }
}

fn get_config(appdir: &Path, workdir: &Path) -> Result<Ini> {
    let cfg_name = "cur_holiday_notification.cfg";
    let mut cfg_file = workdir.join(cfg_name);
    if !cfg_file.exists() {
        cfg_file = appdir.join(cfg_name);
        if !cfg_file.exists() {
            return Err(anyhow!("# Конфиг {} не найден", cfg_name));
        }
    }

    let file = File::open(&cfg_file)?;
    let mut transcoded = DecodeReaderBytesBuilder::new()
        .encoding(Some(WINDOWS_1251))
        .build(file);
    let mut contents = String::new();
    transcoded.read_to_string(&mut contents)?;

    let mut config = Ini::new();
    config.read(contents).map_err(|e| anyhow!("{}", e))?;

    println!("# Используем конфиг: {}", cfg_file.display());
    Ok(config)
}

fn process_xml(config: &Ini, xml_path: &Path) -> Result<Vec<(String, String)>> {
    println!("# Обработка: {}", xml_path.display());

    let fhd = config
        .getbool("features", "HDay")
        .ok()
        .flatten()
        .unwrap_or(false);
    let items = config_items(config, "items");

    let mut results = Vec::new();
    let file = File::open(xml_path)?;
    let mut reader = Reader::from_reader(BufReader::new(file));
    reader.trim_text(true);
    let mut buf = Vec::new();

    let mut report_date = String::new();

    loop {
        match reader.read_event_into(&mut buf) {
            Ok(Event::Start(e)) => {
                let tag = e.name();
                if tag == QName(b"CUX50") {
                    for attr in e.attributes().with_checks(false) {
                        let attr = attr?;
                        if attr.key == QName(b"ReportDate") {
                            report_date = String::from_utf8_lossy(&attr.value).to_string();
                            break;
                        }
                    }
                } else if tag == QName(b"GROUP") && fhd {
                    for attr in e.attributes().with_checks(false) {
                        let attr = attr?;
                        if attr.key == QName(b"TradeGroup") {
                            if &*attr.value == b"H" {
                                results.push(("HDay".to_string(), report_date.clone()));
                            }
                            break;
                        }
                    }
                }
            }
            Ok(Event::Empty(ref e)) => {
                let tag = e.name();
                if tag == QName(b"SECURITY") {
                    for attr in e.attributes().with_checks(false) {
                        let attr = attr?;
                        if attr.key == QName(b"SecShortName") {
                            let name = String::from_utf8_lossy(&attr.value).to_string();
                            for (k, v) in &items {
                                if k.eq_ignore_ascii_case(&name) {
                                    results.push((v.clone(), report_date.clone()));
                                }
                            }
                            break;
                        }
                    }
                }
            }
            Ok(Event::End(e)) if e.name() == QName(b"CUX50") => {
                report_date.clear();
            }
            Ok(Event::Eof) => break,
            Err(e) => return Err(anyhow!("XML Error: {}", e)),
            _ => (),
        }
        buf.clear();
    }

    Ok(results)
}

fn write_results(results: &[(String, String)], workdir: &Path) -> Result<()> {
    let out_path = workdir.join("rust_project.out");
    let mut file = File::create(&out_path)?;
    file.set_len(0)?;

    if results.is_empty() {
        println!("# Нет результата");
    } else {
        println!("# Результат:");
        for (k, v) in results {
            writeln!(file, "{} = {}", k, v)?;
            println!("{} = {}", k, v);
        }
    }

    println!("# Результат сохранен в файл: {}", out_path.display());
    Ok(())
}

fn main() -> Result<()> {
    let appdir = env::current_exe()?.parent().unwrap().to_path_buf();
    let workdir = env::current_dir()?;
    println!("# Директория приложения: {}", appdir.display());
    println!("# Рабочая директория: {}", workdir.display());

    let result = std::panic::catch_unwind(|| {
        if let Err(err) = main_process(&appdir, &workdir) {
            eprintln!("# Программа прервана по ошибке: {:?}", err);
        }
    });

    let args: Vec<String> = env::args().collect();
    let f_mode = if args.len() > 2 {
        args[2] == "true"
    } else {
        false
    };

    if !f_mode {
        println!("# Нажмите <Enter> для выхода");
        let _ = io::stdin().lock().lines().next();
    }

    if result.is_err() {
        std::process::exit(1);
    }

    Ok(())
}
