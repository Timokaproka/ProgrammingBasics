#include <conio.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define BUFFER_SIZE 256  // размер буфера для считывания в файлах
#define READ_CHUNK_SIZE 8192

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
  printf("Введите своё имя (до 32 символов): ");
  if (fgets(username, sizeof(username), stdin) != NULL) {
    // Удаление символа перевода строки, если он попал в буфер
    username[strcspn(username, "\r\n")] = '\0';
  }
  clearing_buffer();  // очищаем буфер
  // Великий Тимофей Владиславович сказал, что clearing_buffer как отдельная функция не нужна, а в итоге то она пригодидлась, зря убирал её(
}

int safe_fopen(FILE** file, const char* filename, const char* mode) {  // <- указатель на указатель 🥶
  if (file == NULL) {
    return EINVAL;  // <- предположим, что это важно. Наверное, потому что память вещь опасная и выходить за рамки своей не очень хочется
  }

  errno = 0;                      // Сбрасываем ошибку перед вызовом (вдруг там есть старая)
  *file = fopen(filename, mode);  // тут уже разыменовали указатель и теперь это просто указатель на экземпляр FILE

  if (*file == NULL) {
    return (errno != 0) ? errno : EIO;  // <- возвращаем ошибку. Если errno осталось 0, но файл мы так и не смогли прочитать, то вызываем "обощённую ошибку ввода-вывода"
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
    bool line_is_ended = (strchr(line, '\n') != NULL);
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

// 9-тый (мой) вариант

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
      printf("Найдены ID: %d, %d\n", i, i + 1);
      neighbours = true;
    }
  }

  if (!neighbours) {
    puts("Предметы не лежат рядом");
  }

  pause_screen();
}

void main_menu() {
  while (true) {
    system("cls");
    printf("%s\n", username);
    puts("[0] Выход");
    puts("[1] Посмотреть на часы");
    puts("[2] Промотать время (поработать)");
    puts("[3] Посмотреть инвентарь");
    puts("[4] Положить предмет в слот");
    puts("[5] Выбросить предмет");
    puts("[6] Соседние ячейки");
    printf("\nВыберите пункт: ");

    switch (interval(0, 6)) {
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
    }
  }
}

int main(int argc, char* argv[]) {
  SetConsoleOutputCP(65001);  // нужно исключительно для винды, потому что ру текст плохо отображается
  // get_username();
  // main_menu();
  // items_list();
  // items_list_export();
  items_list_import();
  for (int i = 0; i < total_number_of_items; i++) {
    printf("%s", items_names[i]);
  }
  pause_screen();
  return EXIT_SUCCESS;
}
