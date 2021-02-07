С и С++ версия загрузчика конфигурационных файлов, чур сильно не пинать,
делал для своих нужд

Разделён на 2 модуля:
1. Загрузчик файла '__config_loader': очищает от комментариев и удаляет пустые строки
2. Интерпритатор '__config_parser': строит структуру Группы[Группа][Переменная].Значение(.Val)

    struct __config_loader {
        /** Методы */
        void (*destruct)(struct __config_loader *I);
        void (*reset)(struct __config_loader *I);
        int (*load)(struct __config_loader *Inst, const char *FileName);
        int (*set_string_comments)(struct __config_loader *I, const char* Keys);
        void (*clear)(struct __config_loader *I);
        int (*isInit)(struct __config_loader *I);
        void (*perror)(struct __config_loader *Inst, const char *What);

        /** Атрибуты(Поля) */
        struct __config_loader_buffers Buffers;
        struct __config_loader_charset Charset;
        struct __config_loader_errors Errors;
        const char *const pBuffer;
        const size_t *Size;
    };

    Загрузчик распознаёт комментарии "/* в стиле Си */", двойную косую "//", шарп "#" и точку с запятой ";"
    Общепринятый формат ".cfg" допускает только строковые комментарии обозначенные символами '#' или ';',
    но для возможности использования загрузчика с другими форматами также удаляются и "Си" комментарии

Создание загрузчика:
    Создание объекта класса __config_loader производится следующим образом:
        struct __config_loader *ConfigLoader = config_loader_construct(0);
    В случае успеха возвращается указатель на объект, в противном - ((struct __config_loader *)0)

Завершение работы с загрузчтком:
    После завершения работы с загрузчиком необходимо воспользоваться деструктором:
        ConfigLoader->destruct(ConfigLoader);

Имеется возможность установить собственный набор символов обозначающих строковый коментарий:
    ConfigLoader->set_string_comments("#");

Загрузка файла:
    Производится вызовом ConfigLoader->load(ConfigLoader, "FileName").
    В случае удачи метод возвращает 0, в противном - -1
    после успешной загрузки инициализируется указатель: const char *const __config_loader::Result,
    а параметр: const size_t Size показывает размер результирующей строки.