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
        struct task* child;
        struct task* next;
};


void print_tree(struct task* element, size_t depth);
void clear_stdin();

void element_add_back_next(struct task** old_element, char* text);
struct task* element_create(const char* text);
void element_destroy(struct task* element);
struct task* element_last(struct task* element);
size_t element_length(const struct task* element);
void remove_from_list(struct task** head, size_t index);
struct task* element_at(struct task* element, size_t id);
struct task* find_by_title(struct task* node, const char* title);
void add_child_to_parent(struct task* root, const char* parent_title, const char* child_title);

// вспомогательная функция для записи из файла
struct task* create_node_from_title(const char* title) {
    if (!title) return NULL;
    struct task* node = malloc(sizeof(struct task));
    if (!node) return NULL;

    size_t len = strlen(title);

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

//создание элемента
struct task* element_create(const char* text){
        // создание элемента
        struct task* element = malloc(sizeof(struct task));
        if(element == NULL){
                printf("Не удалось создать элемент\n");
                return NULL;
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




//добавление элемента в конец вбок
void element_add_back_next(struct task** old_element, char* text){
        struct task* new_element = element_create(text);
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


// удаление элемента 
void remove_child_task(struct task* root, const char* parent_title, const char* child_title){
	struct task* parent = find_by_title(root, parent_title);
	if(!parent){
		printf("Родитель '%s' не найден.\n", parent_title);
    return;
	}

	    // Ищем индекс ребёнка
  size_t index = 0;
  struct task* child = parent->child;
	// используем низкоуровневую функцию удаления по индексу
  while (child) {
       if (strcmp(child->title, child_title) == 0) {
          remove_from_list(&parent->child, index);
          return;
			 }
       child = child->next;
       index++;
  }
  printf("Ребёнок '%s' не найден у '%s'.\n", child_title, parent_title);

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


// новая функция нахождения элемента по имени через рекурсию
struct task* find_by_title(struct task* node, const char* title) {
    if (!node) return NULL;
    if (strcmp(node->title, title) == 0) {
        return node;
    }
    // Ищем в детях
    struct task* child_result = find_by_title(node->child, title);
    if (child_result) return child_result;
    // Ищем в соседях
    return find_by_title(node->next, title);
}

// универсальная функция добавления дитя в родителя
void add_child_to_parent(struct task* root, const char* parent_title, const char* child_title) {
    struct task* parent = find_by_title(root, parent_title);
    if (!parent) {
        printf("Родитель '%s' не найден.\n", parent_title);
        return;
    }
    struct task* new_child = element_create(child_title);
    if (new_child) {
        // Добавляем в КОНЕЦ списка детей (как child_add_back, но с готовым узлом)
        if (parent->child == NULL) {
            parent->child = new_child;
        } else {
            struct task* last = element_last(parent->child);
            last->next = new_child;
        }
    }
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

void parser(struct task** root ,char** tokens, size_t count){
	int16_t in_index = -1;
	for(size_t i = 0; i < count; i++){
		if(strcmp(tokens[i], "в") == 0){
			in_index = i;
			break;
		}
	}
	if(strcmp(tokens[0], "добавить") == 0 && in_index != -1){
		if(in_index > 1 && in_index < count-1){
			const char* parent = tokens[count-1];
			for(size_t i = 1; i < in_index; i++){
				add_child_to_parent(*root, parent, tokens[i]);
			}
		}
	}
	else if(strcmp(tokens[0], "добавить") == 0){
		for(size_t i = 1; i < count; i++){
			element_add_back_next(root, tokens[i]);
		}
	}
	else if(strcmp(tokens[0], "удалить") == 0 && in_index != -1){
		if(in_index > 1 && in_index < count-1){
			const char* parent = tokens[count-1];
			for(size_t i = 1; i < in_index; i++){
				remove_child_task(*root, parent, tokens[i]);
			}
		}
	}
	else if(strcmp(tokens[0], "удалить") == 0){
		for(size_t i = 1; i < count; i++){
		  size_t index = 0;
			struct task* current = *root;
			bool flag = false;
			while (current) {
				  if (strcmp(current->title, tokens[i]) == 0) {
					    remove_from_list(root, index);
							flag = true;
						  break;
				  }
					current = current->next;
					index++;
			}
			if(!flag){
				printf("Корневой элемент '%s' не найден.\n", tokens[1]);
			}
		}
	}
	if(strcmp(tokens[0], "показать") == 0 && strcmp(tokens[1], "план") == 0){
		print_tree(*root, (size_t)0);		
		return;
	}
	return;

}

void start(){
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
	// Будет ли пользователь сохранять в файл
	setup_logging();

	bool run = true;

	while(run){
		// Разбивание строки на токены
		printf("> ");
		char input[256];
		fgets(input, sizeof(input), stdin);
		input[strcspn(input, "\n")] = '\0';

		// Копия для strtok
		char buffer[256];
		strcpy(buffer, input);

		char* tokens[10];
		size_t count = 0;
		char* tok = strtok(buffer, " \t");
		while (tok && count < 10) {
			tokens[count++] = tok;
			tok = strtok(NULL, " \t");
		}
		if(count == 0){
			continue;	
		}else if(count == 1 && strcmp(tokens[0], "выход") == 0){
			run = false;
		}else {
			parser(&root, tokens, count);
		}
	}

	print_tree(root, 0);
	log_tree(root, 0);
  element_destroy(root);
}

int main(){
	start();
	return 0;
	
}

