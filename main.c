#include <conio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winnt.h>

#define INVENTORY_SIZE 10         // размер инвентаря
#define TOTAL_NUMBER_OF_ITEMS 10  // просто кол-во предметов, которые есть. Вообще наверное стоит сделать список или enum, но пока пофик.

int current_day = 1;  // Это и объявление и инициализация (а значит и определение)
int current_hour = 8;
int inventory[INVENTORY_SIZE] = {7, 2, 9, 2, 4, 1, 8, 3, 6, 5};  // инвентарь

// А ТУТ МНЕ GEMINI ВООБЩЕ ГЕНИАЛЬНУЮ ИДЕЮ ПОДСКАЗАЛ. Кароче: const char *
// будет брать начальный БАЙТ строки из ПАМЯТИ. Любая строка заканчивается на
// \0 (вроде нулевой символ), т.к. это просто массив из char. Тот же printf
// БУДЕТ БРАТЬ АДРЕС ПЕРВОГО СИМВОЛА И идти до конца строки (до \0). Так что
// всё будет просто чикибамбони и в итоге будет выводиться вся строка.
// А вот [i] тут выступает просто в роли жёсткой привязки порядкового номера. Кароче ITEM_NAMES[0] всегда будет "Пусто" (точнее указатель на первый байт в патями этой строки... и т.д. и т.п.)
// в независимости от того, куда я запиху этот [0] = "Пусто", хоть на последнее место.
const char* const ITEM_NAMES[TOTAL_NUMBER_OF_ITEMS] = {
    [0] = "Пусто",
    [1] = "Дерево",
    [2] = "Камень",
    [3] = "Семена",
    [4] = "Железо",
    [5] = "Яблоко",
    [6] = "Деньга",
    [7] = "Бутылка воды",
    [8] = "Веревка",
    [9] = "Удочка"};

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
    clearing_buffer();  // очищаем буфер от всего ненужного и ужасного (типа переноса строк)
    puts("Введено не только число");
  }
}

int interval(int min, int max) {
  {
    if (min > max) {
      int temp = max;
      max = min;
      min = temp;
    }
  }
  while (true) {
    int result = int_extractor();
    if (result >= min && result <= max) {
      return result;
    }
    printf("Число должно быть от %d до %d!\n", min, max);
  }
}

void change_time() {
  system("cls");
  puts("РАБОТА\n");
  puts("Сколько часов добавим?");
  int time_add = abs(int_extractor());
  current_hour += time_add;
  current_day += current_hour / 24;
  current_hour = current_hour % 24;
  pause_screen();
}

void set_item_in_slot(int slot, int item_id) {  // вообще думал, что для remove_item буду использовать give_item, с необязательными аргументами,
                                                // но C сказал, что я казуал и аргумент или есть или нет, так что пришлось делать интерфейс (вроде так называется)
                                                // кароче не будьте казуалами и делайте нормально

  if ((slot >= 0 && slot < INVENTORY_SIZE) && (item_id >= 0 && item_id < TOTAL_NUMBER_OF_ITEMS)) {  // если когда-нибудь забуду, добавить проверку для аргументов, при вызове функции
    inventory[slot] = item_id;
  }
}

void give_item() {  // выдача предмета
  system("cls");
  puts("ВЫДАЧА ПРЕДМЕТА\n");
  puts("В какой слот?");
  int slot = interval(0, INVENTORY_SIZE - 1);

  if (inventory[slot] != 0) {
    puts("Слот занят. Сначала выброси предмет из него");
    pause_screen();
    return;
  }

  puts("Что надо?");
  int item_id = interval(1, TOTAL_NUMBER_OF_ITEMS - 1); // Зачем в пустой слот опять ложить воздух 🤔
  set_item_in_slot(slot, item_id);
  pause_screen();
}

void remove_item() {  // удалить предмет из слота
  system("cls");
  puts("УДАЛЕНИЕ ПРЕДМЕТА\n");
  puts("Давай слот");
  int slot = interval(0, INVENTORY_SIZE - 1);
  set_item_in_slot(slot, 0);  // 0 - есть отсутствие предмета в слоте
  pause_screen();
}
// ВСЁ ДАЛЬШЕ ДЛЯ 6-го пункта!!
void count_item() {
  system("cls");
  puts("РЕВИЗИЯ РЕСУРСОВ\n");
  printf("Введите ID предмета (от %d до %d): ", 0, TOTAL_NUMBER_OF_ITEMS - 1);
  int item_id = interval(1, TOTAL_NUMBER_OF_ITEMS - 1);
  int count_item = 0;
  int slots[INVENTORY_SIZE] = {0};
  for (int i = 0; i < INVENTORY_SIZE; i++) {
    if (inventory[i] == item_id) {
      slots[count_item] = i;
      count_item += 1;
    }
  }
  printf("\nКол-во: %d\n", count_item);
  printf("Слоты: ");
  for (int i = 0; i < count_item; i++) {
    printf("%d ", slots[i]);
  }
  puts("");
  pause_screen();
}

void sort_inv() {
  system("cls");
  puts("СОРТИРОВКА ИНВЕНТАРЬ\n");
  int temp_inv[INVENTORY_SIZE] = {0};
  int temp_item = 0;
  for (int i = 0; i < INVENTORY_SIZE; i++) {
    if (inventory[i] != 0) {
      temp_inv[temp_item] = inventory[i];
      temp_item += 1;
    }
  }
  printf("Старый\tНовый\n");
  for (int i = 0; i < INVENTORY_SIZE; i++) {
    printf("%d\t%d\n", inventory[i], temp_inv[i]);
    inventory[i] = temp_inv[i];
  }
  pause_screen();
}

void inversion_inv() {
  system("cls");
  puts("ИНВЕРСИЯ ИНВЕНТАРЬ\n");
  for (int i = 0; i < INVENTORY_SIZE / 2; i++) {
    int temp_item = inventory[i];
    inventory[i] = inventory[INVENTORY_SIZE - 1 - i];
    inventory[INVENTORY_SIZE - 1 - i] = temp_item;
  }
  puts("Инвентарь перевёрнут");
  pause_screen();
}

void unique_items_list() {
  system("cls");
  puts("УНИКАЛЬНОСТЬ ПРЕДМЕТОВ\n");
  int dict_ID_count[TOTAL_NUMBER_OF_ITEMS] = {0};
  for (int i = 0; i < INVENTORY_SIZE; i++) {
    if (inventory[i] != 0) {
      dict_ID_count[inventory[i]] += 1;
    }
  }
  for (int j = 1; j < INVENTORY_SIZE; j++) {
    printf("%s (%d): %d\n", ITEM_NAMES[j], j, dict_ID_count[j]);
  }
  pause_screen();
}

void remove_trash() {
  system("cls");
  puts("ОЧИСТКА ОТ МУСОРА\n");
  printf("Введите ID предмета (от %d до %d): ", 1, TOTAL_NUMBER_OF_ITEMS - 1);
  int item_id = interval(1, TOTAL_NUMBER_OF_ITEMS - 1);
  int count_trash_item = 0;
  for (int i = 0; i < INVENTORY_SIZE; i++) {
    if (inventory[i] == item_id) {
      count_trash_item += 1;
      set_item_in_slot(i, 0);
    }
  }
  printf("Было очищено %d слотов", count_trash_item);
  pause_screen();
}

void find_heaviness() {
  system("cls");
  puts("ПОИСК ТЯЖЕСТЕЙ\n");
  printf("Введите ID предмета (от %d до %d): ", 0, TOTAL_NUMBER_OF_ITEMS - 1);

  int item_id = interval(0, TOTAL_NUMBER_OF_ITEMS - 1);
  for (int i = 0; i < INVENTORY_SIZE; i++) {
    if (inventory[i] > item_id) {
      printf("%d\n", i);
    }
  }
  pause_screen();
}

// Вообще по хорошему тут я должен не дублировать код из unique_items() чтобы получить dict_ID_count (кол-во предметов с ID != 0),
// но нормально реализовать это без структур, указателей и дтинамического выделения памяти невозможно (как утверждает gemini).
// Так что придётся просто дублировать код.

void favorite_item() {
  system("cls");
  puts("ЛЮБИМЫЙ ПРЕДМЕТ\n");
  int max_id = 0;
  int max_count = 0;
  // Собираю кол-во предметов из инвентаря
  int dict_ID_count[TOTAL_NUMBER_OF_ITEMS] = {0};
  for (int i = 0; i < INVENTORY_SIZE; i++) {
    if (inventory[i] != 0) {
      dict_ID_count[inventory[i]] += 1;
    }
  }

  for (int i = 1; i < INVENTORY_SIZE; i++) {
    if (max_count < dict_ID_count[i]) {
      max_count = dict_ID_count[i];
      max_id = i;
    }
  }

  printf("Любимый предмет: %s(%d) - %d шт.\n", ITEM_NAMES[max_id], max_id, max_count);
  pause_screen();
}

void swap_item() {
  puts("ПЕРЕНОС ПРЕДМЕТА В БЫСТРЫЙ СЛОТ\n");
  printf("Введите ID предмета (от %d до %d): ", 0, TOTAL_NUMBER_OF_ITEMS - 1);
  int item_id = interval(1, TOTAL_NUMBER_OF_ITEMS - 1);
  int first_slot = -1;
  for (int i = 0; i < INVENTORY_SIZE; i++) {
    if (inventory[i] == item_id) {
      first_slot = i;
      break;
    }
  }
  if (first_slot == -1) {
    puts("Нет предмета в инвентаре\n");
    pause_screen();
    return;
  } else if (first_slot == 0) {
    puts("Предмет уже в быстром слоте!\n");
    pause_screen();
    return;
  }
  set_item_in_slot(first_slot, inventory[0]);
  set_item_in_slot(0, item_id);

  for (int i = 0; i < INVENTORY_SIZE; i++) {
    puts("Старый\tНовый\n");
    if (i == 0) {
      printf("%s(%d)\t%s(%d)\n", ITEM_NAMES[inventory[first_slot]], inventory[first_slot], ITEM_NAMES[inventory[i]], inventory[i]);
    } else if (i == first_slot) {
      printf("%s(%d)\t%s(%d)\n", ITEM_NAMES[inventory[0]], inventory[0], ITEM_NAMES[inventory[i]], inventory[i]);
    }
    printf("%s(%d)\t%s(%d)\n", ITEM_NAMES[inventory[i]], inventory[i], ITEM_NAMES[inventory[i]], inventory[i]);
  }
  pause_screen();
}

void items_is_neighbours() {
  system("cls");
  puts("ПРОВЕРКА СОСЕДЕЙ ПО ID\n");
  printf("Первый ID (от 0 до %d): ", TOTAL_NUMBER_OF_ITEMS - 1);
  int first_id = interval(0, TOTAL_NUMBER_OF_ITEMS - 1);
  printf("Второй ID (от 0 до %d): ", TOTAL_NUMBER_OF_ITEMS - 1);
  int second_id = interval(0, TOTAL_NUMBER_OF_ITEMS - 1);

  bool neighbours = false;
  // Вот тут надо INVENTORY_SIZE - 1 т.к. будем идти по индексам до i + 1 и чтобы не выходить за границы массива мы будем умными (логика 6-тилетнего ребёнка)
  for (int i = 0; i < INVENTORY_SIZE - 1; i++) {
    if ((inventory[i] == first_id & inventory[i + 1] == second_id) | (inventory[i] == second_id & inventory[i + 1] == first_id)) {  // Тут буду использовать именно побитовые операции, т.к. неизвестно
                                                                                                                                    // как пользователь (любый преподаватель) исказит изначальный инвентарь, так что буду
                                                                                                                                    // использовать вариант без ветвлений.
                                                                                                                                    // На самом деле очень интересная тема с тем, как компилятор преобразует это условие
                                                                                                                                    // с условными операторами и битовыми операторами. Возможно, стоит пересмотреть другие
                                                                                                                                    // условия из уже реализованных функций 🤔
      printf("Найдены ID: %d, %d\n", i, i + 1);
      neighbours = true;
    }
  }
  if (!neighbours) {
    puts("Предметы не лежат рядом");
  }
  pause_screen();
}

// И опять мне придётся использовать dict_ID_count (но может есть вариант лучше).
void remove_duplicates_items() {
  system("cls");
  puts("ТОЛЬКО УНИКАЛЬНЫЕ ПРЕДМЕТЫ\n");
  int dict_ID_count[TOTAL_NUMBER_OF_ITEMS] = {0};
  // Это просто сохраняет старый инвентарь. Для вывода в конче
  int old_inventory[INVENTORY_SIZE];
  for (int x = 0; x < INVENTORY_SIZE; x++) {
    old_inventory[x] = inventory[x];
  }

  // Получаем кол-во предметов по ID
  for (int i = 0; i < INVENTORY_SIZE; i++) {
    if (inventory[i] != 0) {
      dict_ID_count[inventory[i]] += 1;
    }
  }

  // Удаляем предмет из слота, если кол-во предмета > 1
  // и соответственно уменьшаем кол-во предмета в dict_ID_count
  for (int i = 0; i < INVENTORY_SIZE; i++) {
    if (dict_ID_count[inventory[i]] > 1) {
      dict_ID_count[inventory[i]] -= 1;
      inventory[i] = 0;
    }
  }
  puts("Старый\tНовый\n");
  for (int j = 0; j < INVENTORY_SIZE; j++) {
    printf("%s(%d)\t%s(%d)\n", ITEM_NAMES[old_inventory[j]], old_inventory[j], ITEM_NAMES[inventory[j]], inventory[j]);
  }
  pause_screen();
}

void second_menu() {
  while (true) {
    system("cls");
    puts("[0] В главное меню");
    puts("[1] Ревизия ресурсов");
    puts("[2] Сортировка инвентаря");
    puts("[3] Инверсия инвентаря");
    puts("[4] Уникальные предметы");
    puts("[5] Выбросить мусор");
    puts("[6] Поиск тяжестей");
    puts("[7] Любимый ресурс");
    puts("[8] Быстрый доступ");
    puts("[9] Соседние ячейки");
    puts("[10] Удалить дубликаты");
    printf("\nВыберите пункт: ");
    switch (interval(0, 10)) {
      case 0:
        return;
      case 1:
        count_item();
        break;
      case 2:
        sort_inv();
        break;
      case 3:
        inversion_inv();
        break;
      case 4:
        unique_items_list();
        break;
      case 5:
        remove_trash();
        break;
      case 6:
        find_heaviness();
        break;
      case 7:
        favorite_item();
        break;
      case 8:
        swap_item();
        break;
      case 9:
        items_is_neighbours();
        break;
      case 10:
        remove_duplicates_items();
        break;
    }
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
    puts("[6] По вариантам");
    printf("\nВыберите пункт: ");
    switch (interval(6, 0)) {
      case 0:
        return;
      case 1:
        system("cls");
        printf("Текущее время: День %d, %02d:00\n", current_day, current_hour);
        pause_screen();
        break;
      case 2:
        change_time();
        break;
      case 3:
        system("cls");
        puts("ИНВЕНТАРЬ\n");
        for (int i = 0; i < INVENTORY_SIZE; i++) {
          printf("Слот[%d]: %s (%d)\n", i, ITEM_NAMES[inventory[i]], inventory[i]);  // Когда я вижу, что этот массив указателей реально работает у меня полюция под окном проходит.
        }
        pause_screen();
        break;
      case 4:
        give_item();
        break;
      case 5:
        remove_item();
        break;
      case 6:
        second_menu();
        break;
    }
  }
}

int main(int argc, char* argv[]) {
  SetConsoleOutputCP(65001);  // нужно исключительно для винды, потому что ру текст плохо отображается
  main_menu();
  return EXIT_SUCCESS;
}
