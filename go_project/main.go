package main

import (
	"bufio"
	"encoding/xml"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"
	"time"

	"golang.org/x/text/encoding/charmap"
	"golang.org/x/text/transform"
	"gopkg.in/ini.v1"
)

type Security struct {
	SecShortName string `xml:"SecShortName,attr"`
}

type Group struct {
	TradeGroup string     `xml:"TradeGroup,attr"`
	Securities []Security `xml:"SECURITY"`
}

type CUX50 struct {
	ReportDate string  `xml:"ReportDate,attr"`
	Groups     []Group `xml:"GROUP"`
}

type Root struct {
	CUX50Items []CUX50 `xml:"CUX50"`
}

func print_elapsed_time(text string, start, end time.Time) {
	fmt.Printf("# %s: %.9f сек.\n", text, end.Sub(start).Seconds())
}

func main_process(appdir, workdir string) {
	xmlPath, _ := getXMLFile()
	start := time.Now()

	cfg := getConfig(appdir, workdir)
	end_get_config := time.Now()

	result := processXML(cfg, xmlPath)
	end_process_xml := time.Now()

	writeResults(result, workdir)
	end := time.Now()

	print_elapsed_time("* get config", start, end_get_config)
	print_elapsed_time("* process xml", end_get_config, end_process_xml)
	print_elapsed_time("* process results", end_process_xml, end)
	print_elapsed_time("Обработка завершена", start, end)
}

func getXMLFile() (string, error) {
	if len(os.Args) > 1 {
		if _, err := os.Stat(os.Args[1]); err == nil {
			return filepath.Abs(os.Args[1])
		}
		fmt.Printf("# %s не найден\n", os.Args[1])
		os.Exit(1)
	}

	reader := bufio.NewReader(os.Stdin)
	fmt.Print("# XML файл: ")
	file, _ := reader.ReadString('\n')
	file = strings.TrimSpace(file)
	if file == "" {
		return getXMLFile()
	}
	if _, err := os.Stat(file); os.IsNotExist(err) {
		fmt.Printf("# %s не найден\n", file)
		os.Exit(1)
	}
	return filepath.Abs(file)
}

func getConfig(appdir, workdir string) *ini.File {
	cfgName := "cur_holiday_notification.cfg"
	cfgPath := filepath.Join(workdir, cfgName)
	if _, err := os.Stat(cfgPath); os.IsNotExist(err) {
		cfgPath = filepath.Join(appdir, cfgName)
		if _, err2 := os.Stat(cfgPath); os.IsNotExist(err2) {
			fmt.Printf("# Конфиг %s не найден\n", cfgName)
			os.Exit(1)
		}
	}
	cfg, err := ini.LoadSources(ini.LoadOptions{Insensitive: false}, cfgPath)
	if err != nil {
		fmt.Printf("# Не удалось загрузить конфиг: %s\n", err)
		os.Exit(1)
	}
	fmt.Printf("# Используем конфиг: %s\n", cfgPath)
	return cfg
}

func processXML(cfg *ini.File, xmlPath string) [][2]string {
	fmt.Printf("# Обработка: %s\n", xmlPath)

	file, err := os.Open(xmlPath)
	if err != nil {
		panic(err)
	}
	defer file.Close()

	features := cfg.Section("features")
	hDay := features.Key("HDay").MustBool(false)
	items := cfg.Section("items").KeysHash()

	decoder := transform.NewReader(file, charmap.Windows1251.NewDecoder())
	data, err := io.ReadAll(decoder)
	if err != nil {
		panic(err)
	}

	dataStr := strings.Replace(string(data), `encoding="windows-1251"`, ``, 1)
	data = []byte(dataStr)

	var root Root
	err = xml.Unmarshal(data, &root)
	if err != nil {
		panic(err)
	}

	var result [][2]string
	for _, item := range root.CUX50Items {
		for _, g := range item.Groups {
			if hDay {
				if g.TradeGroup == "H" {
					result = append(result, [2]string{"HDay", item.ReportDate})
					goto nextItem
				}
			}
			for _, sec := range g.Securities {
				for k, v := range items {
					if sec.SecShortName == k {
						result = append(result, [2]string{v, item.ReportDate})
						break
					}
				}
			}
		}
	nextItem:
	}
	return result
}

func writeResults(results [][2]string, workdir string) {
	outFile := filepath.Join(workdir, "go_project.out")
	file, err := os.Create(outFile)
	if err != nil {
		panic(err)
	}
	defer file.Close()

	writer := bufio.NewWriter(file)

	if len(results) > 0 {
		fmt.Println("# Результат:")
		for _, r := range results {
			line := fmt.Sprintf("%s = %s\n", r[0], r[1])
			fmt.Print(line)
			writer.WriteString(line)
		}
	} else {
		fmt.Println("# Нет результата")
	}
	writer.Flush()
	fmt.Printf("# Результат сохранен в файл: %s\n", outFile)
}

func main() {
	appdir, _ := filepath.Abs(filepath.Dir(os.Args[0]))
	workdir, _ := os.Getwd()
	fmt.Printf("# Директория приложения: %s\n", appdir)
	fmt.Printf("# Рабочая директория: %s\n", workdir)

	var isError = true
	var fMode = false
	if len(os.Args) > 2 {
		fMode = os.Args[2] == "true"
	}

	defer func() {
		if !fMode {
			fmt.Print("# Нажмите <Enter> для выхода")
			bufio.NewReader(os.Stdin).ReadBytes('\n')
		}
		if isError {
			os.Exit(1)
		} else {
			os.Exit(0)
		}
	}()

	defer func() {
		if r := recover(); r != nil {
			fmt.Println("# Программа прервана по ошибке:", r)
		}
	}()

	main_process(appdir, workdir)
	isError = false
}
