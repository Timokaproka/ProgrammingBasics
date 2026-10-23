#include <conio.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <winerror.h>
#include <wingdi.h>

#define BUFFER_SIZE 256  // размер буфера для считывания в файлах
#define READ_CHUNK_SIZE 8192
#define MAX_MAP_ROWS 64
#define MAX_MAP_COLS 128

char map[MAX_MAP_ROWS][MAX_MAP_COLS + 1];  // +1 под '\0' в конце каждой строки
int map_rows = 0;
int map_cols = 0;

#define HOURS_IN_DAY 24              // кол-во часов в сутках
#define INVENTORY_SIZE 10            // размер инвентаря
#define STANDART_NUMBER_OF_ITEMS 10  // просто кол-во предметов, которые есть. Вообще наверное стоит сделать список или enum, но пока пофик.
int current_day = 1;                 // Это и объявление и инициализация (а значит и определение)
int current_hour = 8;
int inventory[INVENTORY_SIZE] = {7, 2, 9, 2, 4, 1, 8, 3, 6, 5};  // инвентарь
char username[32 + 1];
// А ТУТ МНЕ GEMINI ВООБЩЕ ГЕНИАЛЬНУЮ ИДЕЮ ПОДСКАЗАЛ. Кароче: const char *
// будет брать начальный БАЙТ строки из ПАМЯТИ. Любая строка заканчивается на
// \0 (вроде нулевой символ), т.к. это просто массив из char. Тот же printf
// БУДЕТ БРАТЬ АДРЕС ПЕРВОГО СИМВОЛА И идти до конца строки (до \0). Так что
// всё будет просто чикибамбони и в итоге будет выводиться вся строка.
// А вот [i] тут выступает просто в роли жёсткой привязки порядкового номера. Кароче STANDART_ITEM_NAMES[0] всегда будет "Пусто" (точнее указатель на первый байт в патями этой строки... и т.д. и т.п.)
// в независимости от того, куда я запиху этот [0] = "Пусто", хоть на последнее место.
const char* const STANDART_ITEM_NAMES[STANDART_NUMBER_OF_ITEMS] = {
    [0] = "Пусто",
    [1] = "Дерево",
    [2] = "Камень",
    [3] = "Семена",
    [4] = "Железо",
    [5] = "Яблоко",
    [6] = "Деньга",
    [7] = "Палка",
    [8] = "Веревка",
    [9] = "Удочка"};

char** items_names = NULL;
int total_number_of_items = 0;
int max_item_name = 0;

void pause_screen() {
  puts("\nНажмите любую клавишу для возврата...");
  _getch();
}

void clearing_buffer() {
  int c;

  while ((c = getchar()) != '\n' && c != EOF) {
    // просто очищаем буфер
  }
}

int int_extractor() {
  int result;
  int item_counter;
  char next;

  while (true) {
    item_counter = scanf("%d%c", &result, &next);

    if (item_counter == EOF) {
      exit(EXIT_FAILURE);  // Gemini мне сказал, что это реально важно, если поток ввода заврешён из вне. Оставлю это так, если дадут по шее - уберу.
    } else if (item_counter == 2 && next == '\n') {
      return result;
    }

    clearing_buffer();

    puts("Введено не только число");
  }
}

int interval(int min, int max) {
  if (min > max) {  // зачем были нужны фигурные скобки? temp и так не будет вне этого if...
    int temp = max;
    max = min;
    min = temp;
  }

  while (true) {
    int result = int_extractor();
    if (result >= min && result <= max) {
      return result;
    }
    printf("Число должно быть от %d до %d!\n", min, max);
  }
}

void get_username() {
  bool valid_name = false;
  while (!(valid_name)) {
    printf("Введите своё имя (до 32 байтов): ");  // пусть пользователь сам считает сколько это в символах.
    if (fgets(username, sizeof(username), stdin) != NULL) {
      size_t len = strcspn(username, "\n");
      username[strcspn(username, "\r")] = '\0';
      if (username[len] == '\n') {
        username[len] = '\0';  // строка прочитана целиком, просто убираем '\n'
        valid_name = username[0] != '\0' ? true : false;
      } else {
        clearing_buffer();  // имя длиннее 32 символов, выкидываем остаток
        puts("Имя слишком длинное");
      }
    } else {
      exit(EXIT_FAILURE);
    }
  }

  // Великий Тимофей Владиславович сказал, что clearing_buffer как отдельная функция не нужна, а в итоге то она пригодидлась, зря убирал её(
}

int safe_fopen(FILE** file, const char* filename, const char* mode) {  // <- указатель на указатель 🥶
  if (file == NULL) {
    return EINVAL;  // <- предположим, что это важно. Наверное, потому что память вещь опасная и выходить за рамки своей не очень хочется
  }

  errno = 0;                      // Сбрасываем ошибку перед вызовом (вдруг там есть старая)
  *file = fopen(filename, mode);  // тут уже разыменовали указатель и теперь это просто указатель на экземпляр FILE

  if (*file == NULL) {
    return (errno != 0) ? errno : EIO;  // <- возвращаем ошибку. Если errno осталось 0, но файл мы так и не смогли прочитать, то вызываем "обощённую ошибку ввода-вывода"
  }
  return 0;  // а тут всё круто
}

int count_lines_in_file(FILE* file) {
  if (file == NULL) {
    return 0;
  }

  char buffer[READ_CHUNK_SIZE];
  int lines = 0;
  size_t bytes_read;
  char last_char = '\n';

  while ((bytes_read = fread(buffer, 1, READ_CHUNK_SIZE, file)) > 0) {
    for (size_t i = 0; i < bytes_read; i++) {
      if (buffer[i] == '\n') {
        lines++;
      }
    }
    last_char = buffer[bytes_read - 1];
  }

  if (last_char != '\n') {
    lines++;
  }

  return lines;
}

bool items_list_export() {
  bool return_status = true;
  FILE* items_file;
  int error_code = safe_fopen(&items_file, "items.txt", "w");

  if (error_code == EACCES) {
    puts("Невозможно создать файл items.txt: ОТКАЗАНО В ДОСТУПЕ");
    return_status = false;
    pause_screen();
  } else if (error_code != 0) {
    return_status = false;
    puts("ОШИБКА ПРИ СОЗДАНИИ ФАЙЛА");
    perror("items.txt");
  }

  for (int i = 0; i < STANDART_NUMBER_OF_ITEMS; i++) {
    char str[BUFFER_SIZE];
    snprintf(str, BUFFER_SIZE, "%d", i);  // <- преобразую в строку i, чтобы нормально записать в файл предметы по индексам (а можно и не писать и привязать номер строки к номеру предмета в массиве)
                                          // Можно, а зачем?
    fputs(str, items_file);
    fputc(' ', items_file);
    fputs(STANDART_ITEM_NAMES[i], items_file);
    fputc('\n', items_file);
  }

  fclose(items_file);

  return return_status;
}

void free_items_names() {
  if (items_names == NULL) {  // нет смысла очищать память
    return;
  }

  for (int i = 0; i < total_number_of_items; i++) {
    free(items_names[i]);   // Осовобождаем память которые были под строки
    items_names[i] = NULL;  // Память освободили и убрали значение из укзаателя
  }

  free(items_names);  // Освобождаем пямять самого указателя на указатели
  items_names = NULL;
  total_number_of_items = 0;  // Обнуляем кол-во предметов
}

void items_list_import() {
  FILE* items_file;
  int error_code = safe_fopen(&items_file, "items.txt", "r");

  switch (error_code) {
    case EACCES: {
      puts("items.txt: ОТКАЗАНО В ДОСТУПЕ");
      pause_screen();
      return;
    }
    case ENOENT: {
      int status = items_list_export();
      if (!status) {
        return;
      }
      if (safe_fopen(&items_file, "items.txt", "r") != 0) {
        return;
      }
      break;
    }
    default:
      if (error_code != 0) {
        return;
      }
      break;
  }

  int number_of_line_in_file = count_lines_in_file(items_file);  // узнаём сколько будем загружать предметов, считая строки (в идеальном мире)

  if (number_of_line_in_file == 0) {
    puts("КАКАЯ-ТО ДРЯНЬ ПРОИЗОШЛА, МНЕ УЖЕ ЛЕНЬ ПИСАТЬ ОБРАБОТЧИК ОШИБОК");
    fclose(items_file);
    return;
  }

  rewind(items_file);  // переводим указатель в файле на начало после count_lines_in_file

  char** temp_items_names = realloc(items_names, number_of_line_in_file * sizeof(char*));
  if (temp_items_names == NULL) {
    perror("ПАМЯТИ ДЛЯ ПРЕДМЕТОВ НЕТ БРО ВСЁ КОНЕЦ");
    free(items_names);
    exit(ENOMEM);
  }
  items_names = temp_items_names;
  total_number_of_items = number_of_line_in_file;

  for (int i = 0; i < total_number_of_items; i++) {
    items_names[i] = NULL;  // <- вот эта хрень вообще адовая. ЭТО СУКА САХАР ДЛЯ *(items_names + i)... просто ужас конченный.
                            // Больше ни где не буду это использовать, потому что мой мозг Python-бедолаги не выдерживает такого синтаксиса
  }

  char line[BUFFER_SIZE];
  bool is_new_line = true;
  char* full_line_ptr;
  long id;
  while (fgets(line, BUFFER_SIZE, items_file) != NULL) {
    bool line_is_ended = strchr(line, '\n') != NULL || feof(items_file);  // <- feof возвращает true, если достигнут конец файла
    line[strcspn(line, "\r\n")] = '\0';
    if (is_new_line) {
      full_line_ptr = NULL;
      char* endptr = NULL;
      errno = 0;
      id = strtol(line, &endptr, 10);

      if (endptr == line) {
        printf("НЕТУ ЧИСЛА В СТРОКЕ, ИДИ ФАЙЛ ПЕРЕДЕЛЫВАЙ: %s", line);
      }

      while (*endptr == ' ') {
        endptr++;
      }

      if (id >= 0 && id < total_number_of_items) {
        if (items_names[id] != NULL) {
          printf("АЛО У ТЕБЯ ПОВТОРЫ ID В ФАЙЛЕ: %ld", id);
          free_items_names();
          exit(ENOMEM);
        }
      } else {
        printf("ЧТО У ТЕБЯ С ID? ИДИ ПЕРЕДЕЛЫВАЙ: %s", line);
        free_items_names();
        exit(ENOMEM);
      }

      if (!line_is_ended) {
        is_new_line = false;
      }

      char* temp_full_line_prt = strdup(endptr);
      if (temp_full_line_prt != NULL) {
        full_line_ptr = temp_full_line_prt;
      } else {
        free_items_names();
        exit(ENOMEM);
      }

    } else {
      if (line_is_ended) {
        is_new_line = true;
      }
      char* temp_full_line_prt = realloc(full_line_ptr, (strlen(full_line_ptr) + strlen(line) + 1));
      if (temp_full_line_prt != NULL) {
        full_line_ptr = temp_full_line_prt;
        strcat(full_line_ptr, line);
      } else {
        free_items_names();
      }
    }

    if (line_is_ended) {
      items_names[id] = full_line_ptr;
    }
  }

  fclose(items_file);

  bool full_id = true;
  for (int i = 0; i < total_number_of_items; i++) {
    if (items_names[i] == NULL) {
      full_id = false;
      printf("ОТСУТСТВУЕТ ID: %d", i);
    }

    size_t lenght = strlen(items_names[i]) + 1;
    if (lenght > max_item_name) {
      max_item_name = lenght;
    }
  }
  if (!full_id) {
    free_items_names();
    pause_screen();
    exit(EINVAL);
  }
}

void change_time() {
  system("cls");
  puts("РАБОТА\n");

  puts("Сколько часов добавим?");
  int time_add = abs(int_extractor());

  // Вот это выглядит как то, что можно сделать в 2 действия, но я не представляю как. А может и нельзя 🤨
  current_hour += time_add;
  current_day += current_hour / HOURS_IN_DAY;
  current_hour = current_hour % HOURS_IN_DAY;

  pause_screen();
}

void view_inventory() {
  system("cls");
  puts("ИНВЕНТАРЬ\n");
  for (int i = 0; i < INVENTORY_SIZE; i++) {
    printf("Слот[%d]: %s(%d)\n", i, STANDART_ITEM_NAMES[inventory[i]], inventory[i]);  // Когда я вижу, что этот массив указателей реально работает у меня полюция под окном проходит.
  }
  pause_screen();
}

void set_item_in_slot(int slot, int item_id) {  // вообще думал, что для remove_item буду использовать give_item, с необязательными аргументами,
                                                // но C сКазал, что я кАзуал и аРгумент или есть или нет, тАк что пришлоСь делать Интерфейс (вроде так называется)
                                                // Кароче не будьте казуалами и делайте нормально

  if ((slot >= 0 && slot < INVENTORY_SIZE) && (item_id >= 0 && item_id < total_number_of_items)) {  // если когда-нибудь забуду, добавить проверку для аргументов, при вызове функции
    inventory[slot] = item_id;
  }
}

void give_item() {  // выдача предмета
  system("cls");
  puts("ВЫДАЧА ПРЕДМЕТА\n");

  puts("В какой слот?");
  printf("Введите номер слота (от %d до %d): ", 0, INVENTORY_SIZE - 1);
  int slot = interval(0, INVENTORY_SIZE - 1);

  // Облегчу жизнь конвееру с этими проверками. Просто уберу это и всё.
  // К тому же в ТЗ (лабораторной работе) не было написано про проверку предмета в слоте.

  // if (inventory[slot] != 0) {
  //   puts("Слот занят. Сначала выброси предмет из него");
  //   pause_screen();
  //   return;
  // }

  puts("Что надо?");
  printf("Введите ID предмета (от %d до %d): ", 0, total_number_of_items - 1);

  int item_id = interval(0, total_number_of_items - 1);  // Зачем в пустой слот опять ложить воздух 🤔. UPD: потому что ты не должен проверять предмет в слоте 🙄🙄🙄 тип бошш челл
  set_item_in_slot(slot, item_id);

  pause_screen();
}

void remove_item() {  // удалить предмет из слота
  system("cls");
  puts("УДАЛЕНИЕ ПРЕДМЕТА\n");

  puts("Давай слот");
  printf("Введите номер слота (от %d до %d): ", 0, INVENTORY_SIZE - 1);
  int slot = interval(0, INVENTORY_SIZE - 1);

  set_item_in_slot(slot, 0);  // 0 - отсутствие предмета в слоте
  pause_screen();
}

// 9-тый (мой) вариант для лабы №2

void items_is_neighbours() {
  system("cls");
  puts("ПРОВЕРКА СОСЕДЕЙ ПО ID\n");

  printf("Первый ID (от 0 до %d): ", total_number_of_items - 1);
  int first_id = interval(0, total_number_of_items - 1);

  printf("Второй ID (от 0 до %d): ", total_number_of_items - 1);
  int second_id = interval(0, total_number_of_items - 1);

  bool neighbours = false;

  // Вот тут надо INVENTORY_SIZE - 1 т.к. будем идти по индексам до i + 1 и чтобы не выходить за границы массива мы будем умными (логика 6-тилетнего ребёнка)
  for (int i = 0; i < INVENTORY_SIZE - 1; i++) {
    bool cond1 = (inventory[i] == first_id && inventory[i + 1] == second_id);
    bool cond2 = (inventory[i] == second_id && inventory[i + 1] == first_id);
    if (cond1 || cond2) {  // Наверное, так будет легче читать условия
      printf("Найдены слоты: %d, %d\n", i, i + 1);
      neighbours = true;
    }
  }

  if (!neighbours) {
    puts("Предметы не лежат рядом");
  }

  pause_screen();
}

void find_item_by_name() {
  system("cls");
  puts("ПОИСК ПО НАЗВАНИЮ ПРЕДМЕТА\n");
  printf("Введите название предмета: ");
  char* str = malloc((max_item_name + 2) * sizeof(char));  // <- +1 нужен, чтобы оставалось место под \n из консоли при вводе
  if (str == NULL) {
    exit(ENOMEM);
  }

  bool str_is_ended;
  if (fgets(str, (max_item_name + 2) * sizeof(char), stdin) != NULL) {
    str_is_ended = (strchr(str, '\n') != NULL);
    if (!str_is_ended) {
      puts("Чёт многовато символов");
      free(str);
      clearing_buffer();
      pause_screen();
      return;
    }
    // Удаление символа перевода строки, если он попал в буфер
    str[strcspn(str, "\r\n")] = '\0';
  } else {
    free(str);
    exit(EINVAL);
  }

  if (!str_is_ended) {
    clearing_buffer();
  }

  bool finded = false;

  for (int i = 0; i < total_number_of_items; i++) {
    if (strcmp(items_names[i], str) == 0) {
      finded = true;
      printf("Найден слот: %d", i);
    }
  }

  if (!finded) {
    puts("Нет такого предмета");
  }
  free(str);
  pause_screen();
}

void diary_export() {
  FILE* diary_file;
  int error_code = safe_fopen(&diary_file, "diary.txt", "a");
  switch (error_code) {
    case EACCES: {
      puts("diary.txt: ОТКАЗАНО В ДОСТУПЕ");
      pause_screen();
      return;
    }
    case ENOENT: {
      puts("diary.txt: ФАЙЛ НЕ НАЙДЕН");
      break;
    }
    default:
      if (error_code != 0) {
        return;
      }
      break;
  }

  fputs("---\n", diary_file);
  fprintf(diary_file, "Имя фермера: %s\n", username);
  fprintf(diary_file, "День %d, %02d:00\n", current_day, current_hour);
  for (int i = 0; i < INVENTORY_SIZE; i++) {
    fprintf(diary_file, "%d: %s\n", i, items_names[inventory[i]]);
  }
  fputc('\n', diary_file);

  fclose(diary_file);
  pause_screen();
}

// 10-тый (мой) вариант для лабы №3 (не стал я делать динамический размер карты, не хочу, но так то стоит...)
// Распаковывает RLE-строку в out. Возвращает ширину или -1 при ошибке.
int rle_decode_line(const char* src, char* out) {
  int len = 0;
  while (*src != '\0' && *src != '\n' && *src != '\r') {
    char* end;
    long count = strtol(src, &end, 10);
    if (end == src || count <= 0 || count > MAX_MAP_COLS - len) {
      return -1;  // нет числа, число <= 0 или строка слишком широкая
    }
    char symbol = *end;
    if (symbol == '\0' || symbol == '\n' || symbol == '\r') {
      return -1;  // число без символа
    }
    for (int i = 0; i < count; i++) {
      out[len] = symbol;
      len++;
    }
    src = end + 1;
  }
  out[len] = '\0';
  return len;
}

bool map_read(FILE* map_file) {
  map_rows = 0;  // если загрузка не удастся, карты не будет
  map_cols = 0;

  char line[BUFFER_SIZE];
  int rows = 0;
  int cols = 0;

  while (fgets(line, BUFFER_SIZE, map_file) != NULL) {
    if (line[0] == '\n' || line[0] == '\r') {
      continue;  // пустая строка
    }
    if (rows == MAX_MAP_ROWS) {
      fprintf(stderr, "Слишком много строк в карте\n");
      return false;
    }
    int len = rle_decode_line(line, map[rows]);
    if (len <= 0) {
      fprintf(stderr, "Ошибка в строке: %s", line);
      return false;
    }
    if (rows > 0 && len != cols) {
      fprintf(stderr, "Ширина строки %d, а должна быть %d: %s", len, cols, line);
      return false;
    }
    cols = len;
    rows++;
  }

  if (rows == 0) {
    fprintf(stderr, "Карта пустая\n");
    return false;
  }
  map_rows = rows;
  map_cols = cols;
  return true;
}

bool map_import() {
  system("cls");
  puts("Карта");

  FILE* map_file;
  int error_code = safe_fopen(&map_file, "input.txt", "r");
  if (error_code != 0) {
    fprintf(stderr, "input.txt: %s\n", strerror(error_code));
    pause_screen();
    return false;
  }

  if (map_read(map_file)) {
    printf("Карта загружена: %d x %d\n", map_cols, map_rows);
  }
  fclose(map_file);  // закрываем в одном месте при любом исходе
  return true;
}

// Пишет карту в любой поток или файл или консоль (или куда захочу боже)
void map_write(FILE* out) {
  for (int i = 0; i < map_rows; i++) {
    fprintf(out, "%s\n", map[i]);
  }
}

void map_export() {
  if (map_rows == 0) {
    puts("Карта не загружена. Ну давай, решай проблемы. Боже, кто из нас парень?");
    pause_screen();
    return;
  }

  FILE* output_file;
  int error_code = safe_fopen(&output_file, "output.txt", "w");
  if (error_code != 0) {
    fprintf(stderr, "output.txt: %s\n", strerror(error_code));
    pause_screen();
    return;
  }

  map_write(output_file);
  fclose(output_file);

  puts("Карта сохранена в output.txt");
  pause_screen();
}

void map_show() {
  if (map_rows == 0) {
    puts("Карта не загружена. Твои проблемы — не мои проблемы");
  } else {
    map_write(stdout);
  }
}



void main_menu() {
  while (true) {
    system("cls");
    puts("[0] Выход");
    puts("[1] Посмотреть на часы");
    puts("[2] Промотать время (поработать)");
    puts("[3] Посмотреть инвентарь");
    puts("[4] Положить предмет в слот");
    puts("[5] Выбросить предмет");
    puts("[6] Соседние ячейки");
    puts("[7] Найти предмет по названию");
    puts("[8] Записать в дневник");
    puts("[9] Загрузить карту");
    printf("\nВыберите пункт: ");

    switch (interval(0, 11)) {
      case 0:
        return;
      case 1:
        system("cls");
        printf("Текущее время: День %d, %02d:00\n", current_day, current_hour);  // не думаю, что стоит выносить это в отдельную функцию
        pause_screen();
        break;
      case 2:
        change_time();
        break;
      case 3:
        view_inventory();
        break;
      case 4:
        give_item();
        break;
      case 5:
        remove_item();
        break;
      case 6:
        items_is_neighbours();
        break;
      case 7:
        find_item_by_name();
        break;
      case 8:
        diary_export();
        break;
      case 9:
        if (map_import()) {
          map_show();
          map_export();
        }
        else {
          pause_screen();
        }
        break;
    }
  }
}

int main(int argc, char* argv[]) {
  SetConsoleOutputCP(65001);  // нужно исключительно для винды, потому что ру текст плохо отображается
  SetConsoleCP(65001);        // а это для ввода
  get_username();
  items_list_import();
  main_menu();
  free_items_names();
  return EXIT_SUCCESS;
}
