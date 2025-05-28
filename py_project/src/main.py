import configparser
import os
import sys
import time
import traceback
# noinspection PyPep8Naming
import xml.etree.ElementTree as ET
from configparser import ConfigParser
from pathlib import Path

if getattr(sys, 'frozen', False):
    os.add_dll_directory(f'{Path(sys.executable).parent}/dll')


# noinspection PyMissingOrEmptyDocstring
def main_process(appdir, workdir):
    xml = get_xml_file()
    start = time.time()
    config = get_config(appdir, workdir)

    print(f'# Обработка: {xml}')

    result = process_xml(config, xml)
    write_results(result, workdir)

    print('# Обработка завершена: {:.6f} сек.'.format(time.time() - start))


# noinspection PyMissingOrEmptyDocstring
def config_items(config: ConfigParser, section='default'):
    try:
        return config.items(section)
    except configparser.Error:
        return []


# noinspection PyMissingOrEmptyDocstring
def get_xml_file():
    if len(sys.argv) > 1:
        file = sys.argv[1]
    else:
        if (file := input('# XML файл: ')) == '':
            return get_xml_file()
    if os.path.exists(file):
        return os.path.abspath(file)
    raise FileNotFoundError(f'# {file} не найден')


# noinspection PyMissingOrEmptyDocstring
def get_config(appdir: str, workdir: str) -> ConfigParser:
    cfg_name = 'cur_holiday_notification.cfg'
    cfg_file = os.path.join(workdir, cfg_name)
    if not os.path.exists(cfg_file):
        cfg_file = os.path.join(appdir, cfg_file)
        if not os.path.exists(cfg_file):
            raise FileNotFoundError(f'# Конфиг {cfg_name} не найден')
    config = ConfigParser()
    config.optionxform = str
    cfg_file = os.path.abspath(cfg_file)
    with open(cfg_file, encoding='cp1251') as fp:
        config.read_file(fp)
    print(f'# Используем конфиг: {cfg_file}')
    return config


# noinspection PyMissingOrEmptyDocstring
def process_xml(config, xml):
    fhd = config.getboolean('features', 'HDay')
    items = config_items(config, 'items')
    result = []
    for item in ET.parse(xml).getroot().findall('./CUX50'):
        if fhd:
            if item.find(f"./GROUP[@TradeGroup='H']") is not None:
                result.append(('HDay', item.get('ReportDate')))
                continue
        for k, v in items:
            if item.find(f".//SECURITY[@SecShortName='{k}']") is not None:
                result.append((v, item.get('ReportDate')))
    return result


# noinspection PyMissingOrEmptyDocstring
def write_results(result, workdir):
    out = os.path.join(workdir, 'py_project.out')
    with open(out, 'w', encoding='cp1251') as of:
        of.truncate()
        if result:
            print(f'# Результат:')
            for ri in result:
                of.write(f'{ri[0]} = {ri[1]}\n')
                print(f'{ri[0]} = {ri[1]}')
        else:
            print('# Нет результата')
        print(f"# Результат сохранен в файл: {out}")


# noinspection PyMissingOrEmptyDocstring
def main():
    appdir = os.path.dirname(sys.argv[0])
    appdir = os.path.abspath(appdir)
    workdir = os.path.abspath(os.curdir)
    print(f'# Директория приложения: {appdir}')
    print(f'# Рабочая директория: {workdir}')

    is_error = True
    f_mode = False
    if len(sys.argv) > 2:
        f_mode = sys.argv[2] == 'true'

    try:
        main_process(appdir, workdir)
        is_error = False
    except FileNotFoundError as ex:
        print(ex)
    except KeyboardInterrupt:
        print('\n# Работа программы прервана досрочно')
    except Exception as ex:
        print('# Не могу создать/записать файл результата')
        err = traceback.format_exc().replace('\n', '\n# ')
        print(f'# Программа прервана по ошибке: {ex}%n {err}')

    try:
        if not f_mode:
            input('# Нажмите <Enter> для выхода')
    except (KeyboardInterrupt, UnicodeDecodeError, EOFError):
        pass
    sys.exit(1 if is_error else 0)


if __name__ == '__main__':
    main()
