#include "cstack.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef int hstack_t;

struct node // структура узел стека
{
    struct node* prev; // указатель на предыдущий элемент в стеке
    unsigned int size;
    char data[0]; // гибкий массив для данных
};

typedef struct node* stack_t;

struct stack_entry
{
    int reserved;
    stack_t hstack; // указатель на вершину стека
};

typedef struct stack_entry stack_entry_t;

struct stack_entries_table
{
    unsigned int size; // размер таблицы записей (стеков)
    stack_entry_t* entries; // указатель на массив записей (стеков)
};

struct stack_entries_table g_table = {0u, NULL}; // глобальный динамический массив указателей на верхушки стека

hstack_t stack_new(void) // функция создания нового стека
{
    if (g_table.size == 0)
    {
        g_table.size = 10; // т.к. количество одновременно обслуживаемых стеков не должно быть меньше 10
        g_table.entries = calloc(g_table.size, sizeof(stack_entry_t));
        if (!g_table.entries)
        {
            return -1; // ошибка выделения памяти
        }
    }

    for (unsigned int i = 0; i < g_table.size; ++i)
    {
        if (!g_table.entries[i].reserved) // ищем свободное место
        {
            g_table.entries[i].reserved = 1;
            g_table.entries[i].hstack = NULL; // стек будет изначально пустой
            return (hstack_t)i;
        }
    }

    unsigned int new_size = g_table.size * 2; // увеличиваем размер таблицы
    stack_entry_t* new_entries = realloc(g_table.entries, new_size * sizeof(stack_entry_t)); // перераспределяю память в таблице
    if (!new_entries)
    {
        return -1; // ошибка выделения памяти
    }

    for (unsigned int i = g_table.size; i < new_size; ++i) // инициализирую новые записи
    {
        new_entries[i].reserved = 0;
        new_entries[i].hstack = NULL;
    }

    g_table.entries = new_entries;
    unsigned int new_index = g_table.size; // первый новый индекс
    g_table.size = new_size;

    g_table.entries[new_index].reserved = 1;
    g_table.entries[new_index].hstack = NULL;

    return (hstack_t)new_index;
}

void stack_free(const hstack_t hstack)
{
    if (hstack < 0  || !g_table.entries[hstack].reserved)
    {
        return;
    }
    stack_t node = g_table.entries[hstack].hstack;
    while (node)
    {
        const stack_t prev = node->prev; // сохраняю указатель на предыдущий узел
        free((void*)node); // освобождаю текущий узел (память для data освобождается внутри node)
        node = prev; // перехожу к следующему узлу
    }

    // освобождаю запись в таблице
    g_table.entries[hstack].reserved = 0;
    g_table.entries[hstack].hstack = NULL;
}

int stack_valid_handler(const hstack_t hstack)
{
    if (hstack < 0  || !g_table.entries[hstack].reserved) // проверяю, что хэндлер существует
    {
        return 1;
    }
    return 0;
}

unsigned int stack_size(const hstack_t hstack)
{
    if (hstack < 0  || !g_table.entries[hstack].reserved)
    {
        return 0;
    }

    stack_t node = g_table.entries[hstack].hstack; // получаю указатель на стек

    // считаю количество элементов в стеке
    unsigned int count = 0;
    while (node)
    {
        count++;
        node = node->prev; // Переходим к предыдущему элементу
    }

    return count; // возвращаю количество элементов в стеке
}

void stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    if (hstack < 0  || !g_table.entries[hstack].reserved) // проверяю, что хэндлер существует
    {
        return;
    }
    if (data_in == NULL || size == 0)
    {
        return; // т.к. данные пусты или размер = 0
    }

    // выделяю память для нового узла
    stack_t new_node = (stack_t)malloc(sizeof(struct node) + size);
    if (!new_node)
    {
        return; // Ошибка выделения памяти
    }

    new_node->size = size;

    char* data = (char*)malloc(size);
    if (data == NULL)
        return;
    memcpy(data, data_in, size);
    new_node->data = data;

    new_node->prev = g_table.entries[hstack].hstack;
    g_table.entries[hstack].hstack = new_node; // обновляю указатель на вершину стека
}
unsigned int stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    if (hstack < 0 || !g_table.entries[hstack].reserved) // проверяю, что хэндлер существует
    {
        return 0;
    }

    stack_t node = g_table.entries[hstack].hstack; // получаю указатель на вершину стека

    if (node == NULL) // проверяю, есть ли элементы в стеке
    {
        return 0; // пустой стек
    }

    unsigned int data_size = node->size; // получаю размер данных в узле

    if (size < data_size)
    {
        return 0; // слишком маленький буфер
    }

    memcpy(data_out, node->data, data_size); // копирую данные в буфер

    g_table.entries[hstack].hstack = node->prev; // обновляю указатель на вершину стека
    free(node); // освобождаю память для узла

    return data_size; // возвращаю размер извлеченных данных
}
