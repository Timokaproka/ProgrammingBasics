#include <conio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define HOURS_IN_DAY 24
#define INVENTORY_SIZE 10         // размер инвентаря
#define TOTAL_NUMBER_OF_ITEMS 10  // просто кол-во предметов, которые есть. Вообще наверное стоит сделать список или enum, но пока пофик.

int current_day = 1;  // Это и объявление и инициализация (а значит и определение)
int current_hour = 8;
int inventory[INVENTORY_SIZE] = {7, 2, 9, 2, 4, 1, 8, 3, 6, 5};  // инвентарь
char username[32 + 1];
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
    [7] = "Палка",
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
    printf("Слот[%d]: %s(%d)\n", i, ITEM_NAMES[inventory[i]], inventory[i]);  // Когда я вижу, что этот массив указателей реально работает у меня полюция под окном проходит.
  }
  pause_screen();
}

void set_item_in_slot(int slot, int item_id) {  // вообще думал, что для remove_item буду использовать give_item, с необязательными аргументами,
                                                // но C сКазал, что я кАзуал и аРгумент или есть или нет, тАк что пришлоСь делать Интерфейс (вроде так называется)
                                                // Кароче не будьте казуалами и делайте нормально

  if ((slot >= 0 && slot < INVENTORY_SIZE) && (item_id >= 0 && item_id < TOTAL_NUMBER_OF_ITEMS)) {  // если когда-нибудь забуду, добавить проверку для аргументов, при вызове функции
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
  printf("Введите ID предмета (от %d до %d): ", 0, TOTAL_NUMBER_OF_ITEMS - 1);

  int item_id = interval(0, TOTAL_NUMBER_OF_ITEMS - 1);  // Зачем в пустой слот опять ложить воздух 🤔. UPD: потому что ты не должен проверять предмет в слоте 🙄🙄🙄 тип бошш челл
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

  printf("Первый ID (от 0 до %d): ", TOTAL_NUMBER_OF_ITEMS - 1);
  int first_id = interval(0, TOTAL_NUMBER_OF_ITEMS - 1);

  printf("Второй ID (от 0 до %d): ", TOTAL_NUMBER_OF_ITEMS - 1);
  int second_id = interval(0, TOTAL_NUMBER_OF_ITEMS - 1);

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
  get_username();
  main_menu();
  return EXIT_SUCCESS;
}
