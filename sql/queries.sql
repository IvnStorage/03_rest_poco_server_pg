-- 1. Регистрация пользователя
INSERT INTO
  users (login, password_hash, first_name, last_name)
VALUES
  ('reader11', 'secret123', 'Kirill', 'Orlov') RETURNING id,
  login,
  first_name,
  last_name,
  created_at;

-- 2. Авторизация пользователя
SELECT
  id,
  login,
  first_name,
  last_name
FROM
  users
WHERE
  login = 'reader01'
  AND password_hash = 'secret123';

-- 3. Создание пользователя
INSERT INTO
  users (login, password_hash, first_name, last_name)
VALUES
  ('reader12', 'secret123', 'Nikita', 'Romanov') RETURNING id,
  login,
  first_name,
  last_name;

-- 4. Поиск пользователя по логину
SELECT
  id,
  login,
  first_name,
  last_name,
  created_at
FROM
  users
WHERE
  login = 'reader03';

-- 5. Поиск пользователей по имени/фамилии
SELECT
  id,
  login,
  first_name,
  last_name
FROM
  users
WHERE
  lower(first_name) LIKE '%' || lower('an') || '%'
  AND lower(last_name) LIKE '%' || lower('ov') || '%'
ORDER BY
  id;

-- 6. Создание книги
INSERT INTO
  books (title, author, publication_year)
VALUES
  ('The Cherry Orchard', 'Anton Chekhov', 1904) RETURNING id,
  title,
  author,
  publication_year,
  available;

-- 7. Поиск книг по названию
SELECT
  id,
  title,
  author,
  publication_year,
  available
FROM
  books
WHERE
  lower(title) LIKE '%' || lower('master') || '%'
ORDER BY
  title;

-- 8. Поиск книг по автору
SELECT
  id,
  title,
  author,
  publication_year,
  available
FROM
  books
WHERE
  lower(author) LIKE '%' || lower('tolstoy') || '%'
ORDER BY
  title;

-- 9. Выдача книги пользователю
BEGIN;

SELECT
  id,
  available
FROM
  books
WHERE
  id = 2 FOR
UPDATE
;

INSERT INTO
  loans (user_id, book_id, issued_at, returned)
VALUES
  (1, 2, NOW(), FALSE);

UPDATE
  books
SET
  available = FALSE
WHERE
  id = 2;

COMMIT;

-- 10. Список выдач пользователя
SELECT
  l.id,
  l.user_id,
  l.book_id,
  b.title,
  b.author,
  l.issued_at,
  l.returned_at,
  l.returned
FROM
  loans l
  JOIN books b ON b.id = l.book_id
WHERE
  l.user_id = 1
ORDER BY
  l.issued_at DESC;

-- 11. Возврат книги
BEGIN;

UPDATE
  loans
SET
  returned = TRUE,
  returned_at = NOW()
WHERE
  id = 1
  AND returned = FALSE RETURNING book_id;

UPDATE
  books
SET
  available = TRUE
WHERE
  id = 1;

COMMIT;

-- 12. Доступные книги
SELECT
  id,
  title,
  author,
  publication_year
FROM
  books
WHERE
  available = TRUE
ORDER BY
  title;

-- 13. Активные выдачи
SELECT
  l.id,
  u.login,
  b.title,
  l.issued_at
FROM
  loans l
  JOIN users u ON u.id = l.user_id
  JOIN books b ON b.id = l.book_id
WHERE
  l.returned = FALSE
ORDER BY
  l.issued_at;

-- 14. EXPLAIN: поиск пользователя по логину
EXPLAIN (ANALYZE, BUFFERS)
SELECT
  id,
  login,
  first_name,
  last_name
FROM
  users
WHERE
  login = 'reader01';

-- 15. EXPLAIN: список выдач пользователя
EXPLAIN (ANALYZE, BUFFERS)
SELECT
  l.id,
  l.user_id,
  l.book_id,
  l.issued_at,
  l.returned_at,
  l.returned
FROM
  loans l
WHERE
  l.user_id = 1
ORDER BY
  l.issued_at DESC;

-- 16. EXPLAIN: поиск книг по названию
EXPLAIN (ANALYZE, BUFFERS)
SELECT
  id,
  title,
  author,
  publication_year,
  available
FROM
  books
WHERE
  lower(title) LIKE '%' || lower('war') || '%';