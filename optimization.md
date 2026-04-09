# Оптимизация запросов

## 1. Поиск пользователя по логину

```sql
SELECT id, login, first_name, last_name
FROM users
WHERE login = 'reader01';
```

Оптимизация:  
у поля login есть UNIQUE => автоматически создаётся индекс.

Результат:  
используется Index Scan вместо полного сканирования таблицы.

---

## 2. Список выдач пользователя

```sql
SELECT l.id, l.user_id, l.book_id, l.issued_at, l.returned_at, l.returned
FROM loans l
WHERE l.user_id = 1
ORDER BY l.issued_at DESC;
```

Оптимизация:

```sql
CREATE INDEX idx_loans_user_issued_at ON loans(user_id, issued_at DESC);
```

Результат:  
данные сразу читаются в нужном порядке, без отдельной сортировки.

---

## 3. Поиск книг по названию

```sql
SELECT id, title, author, publication_year, available
FROM books
WHERE lower(title) LIKE '%war%';
```

Оптимизация:

```sql
CREATE EXTENSION IF NOT EXISTS pg_trgm;
CREATE INDEX idx_books_title_trgm ON books USING gin (lower(title) gin_trgm_ops);
```

Результат:  
ускоряется поиск по подстроке, вместо полного сканирования таблицы.

---

## 4. Поиск книг по автору

```sql
SELECT id, title, author, publication_year, available
FROM books
WHERE lower(author) LIKE '%tolstoy%';
```

Оптимизация:

```sql
CREATE INDEX idx_books_author_trgm ON books USING gin (lower(author) gin_trgm_ops);
```

Результат:  
быстрый поиск по части имени автора.

---

## 5. Поиск пользователей по имени и фамилии

```sql
SELECT id, login, first_name, last_name
FROM users
WHERE lower(first_name) LIKE '%an%'
  AND lower(last_name) LIKE '%ov%';
```

Оптимизация:

```sql
CREATE INDEX idx_users_first_name_trgm ON users USING gin (lower(first_name) gin_trgm_ops);
CREATE INDEX idx_users_last_name_trgm ON users USING gin (lower(last_name) gin_trgm_ops);
```

Результат:  
ускоряется поиск по маске.

---

## 6. Защита от двойной выдачи книги

Оптимизация:

```sql
CREATE UNIQUE INDEX uq_loans_book_active ON loans(book_id) WHERE returned = FALSE;
```

Плюс используется SELECT ... FOR UPDATE в транзакции.

Результат:  
нельзя выдать одну книгу дважды одновременно.

---

## 7. Проверка через EXPLAIN

```sql
EXPLAIN (ANALYZE, BUFFERS)
SELECT id, login FROM users WHERE login = 'reader01';

EXPLAIN (ANALYZE, BUFFERS)
SELECT * FROM loans WHERE user_id = 1 ORDER BY issued_at DESC;

EXPLAIN (ANALYZE, BUFFERS)
SELECT * FROM books WHERE lower(title) LIKE '%war%';
```

Смотрим:
- Seq Scan => Index Scan
- исчезает сортировка
- уменьшается количество операций чтения

---

## 8. Партиционирование

Для данного учебного проекта скорее всего не требуется.

Если данных станет много - можно разбить таблицу loans по месяцам (issued_at).