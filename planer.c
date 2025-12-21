#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define LOG(...) do { \
    if (output_file) { \
        fprintf(output_file, __VA_ARGS__); \
        fflush(output_file); /* сразу пишем на диск */ \
    } \
} while(0)


FILE* output_file = NULL; 

struct task {
        char* title;
        uint8_t is_header;
        struct task* child;
        struct task* next;
};


void user_input();
void print_tree(struct task* element, size_t depth);
void clear_stdin();

struct task* element_create();
void element_destroy(struct task* element);
void element_add_front_next(struct task** old_element);
void element_add_back_next(struct task** old_element);
struct task* element_last(struct task* element);
struct task* element_at(struct task* element, size_t id);
void element_paste(struct task** old_element, size_t index);
size_t element_length(const struct task* element);
void remove_from_list(struct task** head, size_t index);

void child_add_front(struct task* parent);
void child_add_back(struct task* parent);
void child_add_at(struct task* parent, size_t index);

//Вспомогательные функции
void user_input(char* mod, size_t size) {
    if (fgets(mod, (int)size, stdin) == NULL) {
        mod[0] = '\0';
        return;
    }
    // Убираем \n, если есть
    size_t len = strlen(mod);
    if (len > 0 && mod[len - 1] == '\n') {
        mod[len - 1] = '\0';
    } else {
        // Строка не влезла — читаем и игнорируем остаток
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }
}

// вспомогательная функция для записи из файла
struct task* create_node_from_title(const char* title) {
    if (!title) return NULL;
    struct task* node = malloc(sizeof(struct task));
    if (!node) return NULL;

    size_t len = strlen(title);
    node->is_header = (len > 0 && title[len - 1] == '!');

    node->title = malloc(len + 1);
    if (!node->title) {
        free(node);
        return NULL;
    }
    strcpy(node->title, title);

    node->child = NULL;
    node->next = NULL;
    return node;
}

// записывает с файла и создает списки
struct task* load_from_file(char* filename) {
		//сохраняет указатель на файл
    FILE* fp = fopen(filename, "r");
    if (!fp) return NULL;

		//подгатавливаем мини стек
    #define MAX_DEPTH 20
    struct task* stack[MAX_DEPTH] = {NULL};

		//создаем корень списка
    struct task* root = NULL;

		//читаем файл построчно - максимум 512 байт
    char line[512];

		// читаем файл
    while (fgets(line, sizeof(line), fp)) {
				//меняет символы переноса, на завершающую
        line[strcspn(line, "\n")] = '\0';
				//если символ завершающий, то continue
        if (line[0] == '\0') continue;

        // Считаем отступы (4 пробела = 1 уровень)
        int spaces = 0;
        while (line[spaces] == ' ') spaces++;
				//узнаем глубину, через пробелы
        int depth = spaces / 4;
        if (depth >= MAX_DEPTH) continue;
				
				//подгатавливаем строку
        char* title = line + spaces;
				//создаем элемент, через аналогичную функцию
        struct task* node = create_node_from_title(title);
        if (!node) continue;
				
				// если глубина 0 - корень
        if (depth == 0) {
						// если root == NULL, указываем на первый элемент, заносим в стек
            if (!root) {
                root = node;
                stack[0] = node;
            } else {
                struct task* last = root;
                while (last->next) last = last->next;
                last->next = node;
                stack[0] = node;
            }
        } else {
            struct task* parent = stack[depth - 1];
            if (!parent) {
                element_destroy(node); // некорректная строка
                continue;
            }
            if (!parent->child) {
                parent->child = node;
            } else {
                struct task* last_child = element_last(parent->child);
                last_child->next = node;
            }
            stack[depth] = node;
        }
    }

    fclose(fp);
    return root;
}

//создание элемента
struct task* element_create(){
        // создание элемента
        struct task* element = malloc(sizeof(struct task));
        if(element == NULL){
                printf("Не удалось создать элемент\n");
                return NULL;
        }
        //присваивание текста
        char text[1024];
        user_input(text, sizeof(text));

        //проверка на заголовок
        size_t len = strlen(text);
        element->is_header = 0;
        if(len > 0 && text[len - 1] == '!'){
                element->is_header = 1;
        }
        // копируем текст в кучу, чтобы не уничтожался после выхода из функции
        char* text_copy = malloc(strlen(text)+1);
        if(text_copy == NULL){
                printf("Не удалось создать элемент\n");
                free(element);
                return NULL;
        }
        text_copy = strcpy(text_copy, text);

        element->title = text_copy;
        element->next = NULL;
        element->child = NULL;

        return element;
}

// уничтожение рекурсией
void element_destroy(struct task* element){
        if(element == NULL) return;

        element_destroy(element->child);
        element_destroy(element->next);

        free(element->title);
        free(element);
}


// вывод дерева в файл
void log_tree(struct task* element, size_t depth){
	if(element == NULL) return;

	for(size_t i = 0; i < depth; i++){
			LOG("    ");
	}
	LOG("%s\n", element->title);

	log_tree(element->child, depth+1);
	log_tree(element->next, depth);
}

//вывод дерева
void print_tree(struct task* element, size_t depth){
        if(element == NULL){
                return;
        }

        for(size_t i = 0; i < depth; i++){
                printf("    ");
        }
        printf("%s\n", element->title);

        print_tree(element->child, depth+1);
        print_tree(element->next, depth);
}


// добавление элемента в начало вбок
void element_add_front_next(struct task** old_element){
        struct task* new_element = element_create();
        if(new_element == NULL) {
                return;
        }
        new_element->next = *old_element;
        *old_element = new_element;
}


//добавление элемента в конец вбок
void element_add_back_next(struct task** old_element){
        struct task* new_element = element_create();
        if(new_element == NULL) {
					printf("Не удалось создать элемент\n");
					return;
				}
        if(*old_element == NULL){
                *old_element = new_element;
                return;
        }
        struct task* second_element = element_last(*old_element);
				if(second_element == NULL){
					printf("Не удалось найти последний элемент");
					return;
				}
        second_element->next = new_element;
}


// нахождение последнего элемента
struct task* element_last(struct task* element){
        if(element == NULL){
                return NULL;
        }

        while(element != NULL){
                if(element->next == NULL){
                        return element;
                }
                element = element->next;
        }
}

// вставка элемента по индексу
void element_paste(struct task** old_element, size_t index){
        struct task* new_element = element_create();
				if(new_element == NULL) return;
        if(index == 0){
                new_element->next = *old_element;
                *old_element = new_element;
        }
        // находим элемент index-1
        if(index > 0){
                struct task* find_element = element_at(*old_element, index-1);
                if(find_element == NULL) return;
                new_element->next = find_element->next;
                find_element->next = new_element;
        }

}

//удаление корневого элемента
void remove_from_list(struct task** head, size_t index){
	if(*head == NULL){
		return;
	}

	if(index == 0){
		struct task* target = *head;
		*head = target->next;
		element_destroy(target->child);
		free(target->title);
		free(target);
	}
	else{
		struct task* target_pre = element_at(*head, index-1);
		if(target_pre == NULL){
			return;
		}
		if(target_pre->next == NULL){
			return;
		}
		else{
			struct task* target = target_pre->next;
			target_pre->next = target->next;
			element_destroy(target->child);
			free(target->title);
			free(target);
		}

	}
}

// удаление детей
void remove_child_at(struct task* parent, size_t index){
	if(parent == NULL) return;
	if(parent->child == NULL) {
		printf("У этой категории задачи нет:/\n");
		return;
	}
	if(index == 0){
		struct task* target = parent->child;
		parent->child = target->next;
		element_destroy(target->child);
		free(target->title);
		free(target);
	}
	else if(index > 0){
		struct task* target_pre = element_at(parent->child, index-1);
		if(target_pre == NULL) return;
		if(target_pre->next == NULL) return;
		struct task* target = target_pre->next;
		target_pre->next = target->next;
		element_destroy(target->child);
		free(target->title);
		free(target);
	}

}

//добавление детей в начало ветки
void child_add_front(struct task* parent){
	if(parent == NULL) return;
	struct task* child = element_create();
	if(child == NULL){
		return;
	}

	child->next = parent->child;
	parent->child = child;

}

//добавление детей в конец ветки
void child_add_back(struct task* parent){
	if(parent == NULL) return;
	struct task* child = element_create();
	//if(child == NULL){
		//return;
	//}
	if(parent->child == NULL) {
		parent->child = child;
		return;
	}
	struct task* last_child = element_last(parent->child);
	last_child->next = child;
}

//добавление детей в любое место ветки
void child_add_at(struct task* parent, size_t index){
	if(parent == NULL) return;
	struct task* child = element_create();
	if(child == NULL){
		return;
	}

	if(index == 0){
		child->next = parent->child;
		parent->child = child;
	}
	if(index > 0){
    struct task* find_element = element_at(parent->child, index-1);
    if(find_element == NULL) return;
		child->next = find_element->next;
		find_element->next = child;
	}

}



// нахождение элемента
struct task* element_at(struct task* element, size_t id){

        if(element == NULL) return NULL;
        if(id >= element_length(element)) return NULL;

        for(size_t i = 0; element != NULL; i++){
                if(i == id){
                        return element;
                }
                element = element->next;
        }
        printf("Элемент не найден\n");
        return NULL;
}


//расчет колво элементов
size_t element_length(const struct task* element){
        size_t count = 0;
        while(element != NULL){
                count++;
                element = element->next;
        }
        return count;
}

void setup_logging() {
    printf("Сохранять расчёты в файл? (y/n): ");
    char choice;
    scanf(" %c", &choice); // пробел перед %c поглощает \n
    clear_stdin();

    if (choice == 'y' || choice == 'Y') {
        char filename[100];
        printf("Введите имя файла: ");
        fgets(filename, sizeof(filename), stdin);
        filename[strcspn(filename, "\n")] = '\0'; // убрать \n

        // Открываем в режиме "a" (append) — добавляем в конец
        output_file = fopen(filename, "a");
        if (output_file == NULL) {
            printf("Ошибка: не удалось открыть файл '%s'\n", filename);
            output_file = NULL; // на всякий случай
        }
    }
}

void clear_stdin(){
	int c;
	while((c = getchar()) != '\n' && c != EOF);
}

int main(){
	printf("Загрузить с файла? (y/n): ");
	char x;
	scanf("%c", &x);
	clear_stdin();

	struct task* root = NULL;

	if(x == 'y' || x == 'Y'){
		char filename[100];
		printf("Введите имя файла: ");
		fgets(filename, sizeof(filename), stdin);
		clear_stdin();
		filename[strcspn(filename, "\n")] = '\0';
		root = load_from_file(filename);
	}

	setup_logging();
	printf("== Планер Обучения ==\n");

	int8_t choise = 1;
	while(choise){
			if(root == NULL){
				printf("Список пуст. Напишите свою первую категорию.\n");
				struct task* category = element_create();
				if(category && category->is_header){
					root = category;
				}
				else{
					printf("Первый Элемент должен быть категорией( Пример: \"Имя!\" <<)\n");
					if(category) element_destroy(category);
				}

			}
			else{
				  printf("\nВыберите действие:\n");
          printf("1. Добавить новую категорию (вбок)\n");
          printf("2. Добавить задачу в категорию (вглубь)\n");
					printf("3. Удалить категорию/дочернюю категорию\n");
          printf("4. Показать план\n");
          printf("0. Выйти\n");
          printf("> ");
					scanf("%" SCNd8, &choise);
					clear_stdin();

					switch(choise){
						case 1: {
							printf("Введите новую категорию: \n");
							printf("> ");
							element_add_back_next(&root);
							break;
						}
						case 2: {
							printf("Выберите в какую Категорию добавить задачу(отсчет начинатеся с 0)\n ");
							printf("\n");
							print_tree(root, 0);
							printf("\n");
							printf("> ");
							int8_t choise_cat;
							scanf("%" SCNd8, &choise_cat);
							clear_stdin();
							struct task* need_cat = element_at(root, choise_cat);
							if(need_cat == NULL){
								printf("Не удалось найти элемент\n");
								continue;
							}
							printf("Выберете задачу для категории \"%s\": \n", need_cat->title);
							printf("\n");
							print_tree(need_cat->child, 0);
							printf("\n");
							printf("> ");
							child_add_back(need_cat);
							break;
						}
						case 3: {
							printf("Удалить Категорию или Дочернюю категорию(1/2)\n");
							int8_t choise_sec, choise_three;
							printf("> ");
							scanf("%" SCNd8, &choise_sec);
							switch(choise_sec){
									case 1: {
										printf("Выберите Категорию, которую хотите удалить(отсчет начинается с 0)\n");
										printf("\n");
										print_tree(root, 0);
										printf("\n");
										printf("> ");
										scanf("%" SCNd8, &choise_three);
										clear_stdin();
										remove_from_list(&root, choise_three);
										break;
									}
									case 2: {
										printf("Выберите Категорию, в которой хотите удалить элемент(отсчет начинается с 0)\n");
										printf("\n");
										print_tree(root, 0);
										printf("\n");
										printf("> ");
										scanf("%" SCNd8, &choise_three);
										struct task* need_cat = element_at(root, choise_three);
										if(need_cat == NULL){
											printf("Не удалось найти элемент\n");
											continue;
										}
										printf("Выберите Элемент, который хотите удалить(отсчет начинается с 0)\n");
										printf("\n");
										print_tree(need_cat->child, 0);
										printf("\n");
										printf("> ");
										scanf("%" SCNd8, &choise_three);
										remove_child_at(need_cat, choise_three);
										break;
									}

							 	
							}
						}
						case 4: {
							print_tree(root, 0);
							break;
						}
						case 0: {
							choise = 0;
							break;
						}
						default:
							printf("Введите нормально число");
					}
			}
	}
	printf("== Финальный план ==\n");
	print_tree(root, 0);
	log_tree(root, 0);
  if (output_file) {
      fclose(output_file);
  }
	element_destroy(root);
	return 0;
}
