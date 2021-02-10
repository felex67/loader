С и С++ версия загрузчика конфигурационных файлов, чур сильно не пинать,
делал для своих нужд

Разделён на 2 модуля:
1. Загрузчик файла '__config_cleaner': очищает от комментариев и удаляет пустые строки
2. Интерпритатор '__config_parser': строит структуру Группы[Группа][Переменная].Значение(.Val)

##############################################################################################
#                                     1. ConfigCleaner                                       #
##############################################################################################

    typedef struct __config_cleaner {
        void (*const destruct)();               /* Деструктор загрузчика */
        void (*const reset)();                  /* Очищает буферы, устанавливает значения по умолчанию */
        void (*const resetBuff)();              /* очищает только буферы */
        int (*const load)();            /* Загружает и отчищает файл от коментариев, удаляет пустые строки */
        int (*const set_string_comments)();     /* устанавливает типы строчных комментариев */
        int (*const release)();                 /* Отпускает выходной массив */

        const struct __config_cleaner_buffers __private_buffs;      /* Структура массивов */
        const struct __config_cleaner_charset __private_char;       /* Набор символов */
        const size_t __private_flags;       /* Флаги */
    } ConfigCleaner;
    
    /** Конструктор класса */
    ConfigCleaner* new_config_cleaner(ConfigCleaner *);

    Загрузчик распознаёт комментарии "/* в стиле Си */", двойную косую "//", шарп "#" и точку с запятой ";"
    Общепринятый формат ".cfg" допускает только строковые комментарии обозначенные символами '#' или ';',
    но для возможности использования загрузчика с другими форматами также удаляются и "Си" комментарии

Создание загрузчика:
    Создание объекта класса или его инициализация производится вызовом:
        ConfigCleaner* Cleaner = new_config_cleaner(0);
            или
        new_config_cleaner(Cleaner);
    Разница в том что в первом случае объект создаётся в динамической памяти,
    а во втором - инициализируется статичный объект.
    в случае успеха функция возвращает указатель на инициализированный объект,
    в противном - ноль(NULL, nullptr)

Завершение работы с загрузчтком:
    После завершения работы с загрузчиком необходимо воспользоваться деструктором:
        ConfigLoader->destruct(ConfigLoader);

Имеется возможность установить собственный набор символов обозначающих строковый коментарий:
    ConfigLoader->set_string_comments("#");

Загрузка файла:
    Производится вызовом ConfigLoader->load(ConfigLoader, "FileName").
    В случае удачи метод возвращает 0, в противном - -1
    в случае успешной зашрузки доступ к файлу можно получить методом
        ConfigCleaner::release(ConfigCleaner *Inst, void **Tgt, size_t *TgtSize)

##############################################################################################
#                                      2. ConfigParser                                       #
##############################################################################################

    typedef struct __config_parser {
    /** public: */
        void (*const destruct)();                       /** Деструктор */
        void (*const reset)();                          /** Очистка (полная) */
        int (*const init)();                            /** основной рабочий метод */
        int (*const set_building_method)();                     /** Устанавливает метод построения */
        
        int (*const parse_i)();
        int (*const parse_ui)();
        int (*const parse_l)();
        int (*const parse_ul)();
        int (*const parse_f)();
        int (*const parse_d)();
        const char *(*const parse_str)();

    /** private: */
        int (*const __private_count)();
        int (*const __private_build)();
        int (*const __private_init_map)();
        int (*const __private_counter)();
        int (*const __private_builder)();

        const size_t __private_gc;                      /*  Найдено групп */
        const size_t __private_vc;                      /* Найдено переменных */
        const parser_group_t *const __private_map;      /*  Указатель на массив групп */
        const parser_variable_t *const __private_vars;  /* Указатель на массив переменных */
        const unsigned char *const __private_src;       /* Рабочий массив */
        const size_t __private_srcsz;                   /* Размер рабочего массива */
        const struct __config_parser_flags __private_flags;     /* Флаги состояния */
        const int Error;                                /* Ошибки выполнения */
    } ConfigParser;

    Тут всё просто:
    ...
        ConfigParser *Parser = new_config_parser(0);
        Parser->init(Parser, FileName);
        double d;
        Parser->parser_d(Parser, &d, VarName, VarGrp); // Если VarGrp = 0, то выполняется поиск только по массиву переменных
        Parser->destruct(Parser);
    ...
