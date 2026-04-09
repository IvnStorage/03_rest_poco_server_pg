# Домашнее задание 03. Проектирование и оптимизация реляционной базы данных

В рамках работы реализована система управления библиотекой на C++ с использованием Poco и PostgreSQL.

---
## Вариант 15 – Система управления библиотекой

Приложение должно содержать следующие данные:

- Пользователь  
- Книга  
- Выдача  

Необходимо реализовать API:

- Создание нового пользователя  
- Поиск пользователя по логину  
- Поиск пользователя по маске имя и фамилии  
- Добавление книги в библиотеку  
- Поиск книги по названию  
- Поиск книги по автору  
- Создание выдачи книги пользователю  
- Получение списка выданных книг пользователя  
- Возврат книги  

---

## Описание

Сервис позволяет:
- регистрировать и авторизовывать пользователей
- добавлять книги
- искать пользователей и книги
- выдавать книги пользователям
- возвращать книги

Данные хранятся в памяти (in-memory).

## Сущности

- User — пользователь
- Book — книга
- Loan — выдача книги

## Endpoints

### Auth
- POST /api/v1/auth/register — регистрация пользователя
- POST /api/v1/auth/login — логин и получение JWT

### Users
- POST /api/v1/users — создание пользователя
- GET /api/v1/users/by-login/{login} — поиск по логину
- GET /api/v1/users/search — поиск по имени и фамилии

### Books
- POST /api/v1/books — добавить книгу (требует JWT)
- GET /api/v1/books/search — поиск книг

### Loans
- POST /api/v1/loans — выдать книгу (требует JWT)
- GET /api/v1/users/{userId}/loans — список выдач пользователя
- PATCH /api/v1/loans/{loanId}/return — вернуть книгу (требует JWT)

### Service
- GET /metrics — метрики
- GET /swagger.yaml — OpenAPI спецификация

## Аутентификация

Используется JWT.

После логина возвращается токен:

```json
{
  "token": "..."
}
```

Передаётся в заголовке:

Authorization: Bearer <token>

Защищённые endpoint'ы:
- создание книги
- создание выдачи
- возврат книги

## Статус-коды

- 200 OK — успешный запрос
- 201 Created — ресурс создан
- 400 Bad Request — неверные данные
- 401 Unauthorized — нет токена или неверный токен
- 404 Not Found — ресурс не найден
- 409 Conflict — конфликт (например, книга уже выдана)

## Документация API

- файл: openapi.yaml
- endpoint: GET /swagger.yaml

## Переменные окружения

- PORT — порт сервера (по умолчанию 8080)
- LOG_LEVEL — уровень логирования
- JWT_SECRET — секрет для JWT

## Запуск через Docker

Сборка проекта:

```bash
curl "http://localhost:8080/api/v1/users/by-login/reader01"
curl "http://localhost:8080/api/v1/books/search?title=War"
curl "http://localhost:8080/api/v1/books/search?author=Tolstoy"
curl "http://localhost:8080/api/v1/users/1/loans"
```

## Запуск проекта

### Сборка и запуск

```bash
docker compose up --build
```

После запуска:
- API доступен по адресу `http://localhost:8080`;
- PostgreSQL доступен на порту `5432`.

### Остановка и удаление контейнеров

```bash
bash tests/run_api_tests.sh
```

---

## Что проверяют тесты

Тесты покрывают основные сценарии работы API:

- регистрация пользователя
- проверка ошибки при повторной регистрации
- логин и получение JWT токена
- ошибка при неверном пароле
- создание пользователя
- поиск пользователя (по логину и по имени)
- добавление книги (с токеном и без)
- поиск книги
- создание выдачи книги
- запрет повторной выдачи той же книги
- получение списка выдач пользователя
- возврат книги
- запрет повторного возврата
- обработка ошибок (404, 401, 409)

Также тесты проверяют:
- корректные HTTP статус-коды
- корректные JSON-ответы
- работу авторизации через JWT

---

## Пример успешного запуска тестов

```bash
$ bash tests/run_api_tests.sh

==================================================
==> 1. Register first user
==================================================
Request:
  Method: POST
  URL:    http://localhost:8080/api/v1/auth/register
  Token:  no
  Body:
    {
        "login":  "reader_auto_1774680823",
        "password":  "secret123",
        "firstName":  "Ivan",
        "lastName":  "Petrov"
    }

Response:
  Status: 201
  Body:
    {
        "firstName":  "Ivan",
        "id":  1,
        "lastName":  "Petrov",
        "login":  "reader_auto_1774680823"
    }


==================================================
==> 2. Register duplicate user should fail
==================================================
Request:
  Method: POST
  URL:    http://localhost:8080/api/v1/auth/register
  Token:  no
  Body:
    {
        "login":  "reader_auto_1774680823",
        "password":  "secret123",
        "firstName":  "Ivan",
        "lastName":  "Petrov"
    }

Response:
  Status: 409
  Body:
    {
        "error":  "user already exists"
    }


==================================================
==> 3. Login first user
==================================================
Request:
  Method: POST
  URL:    http://localhost:8080/api/v1/auth/login
  Token:  no
  Body:
    {
        "login":  "reader_auto_1774680823",
        "password":  "secret123"
    }

Response:
  Status: 200
  Body:
    {
        "token":  "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE3NzQ2ODA4MjcuMzc0NDcsImxvZ2luIjoicmVhZGVyX2F1dG9fMTc3NDY4MDgyMyIsInN1YiI6InJlYWRlcl9hdXRvXzE3NzQ2ODA4MjMiLCJ1c2VySWQiOjF9.gjSP-q84o8Iq6ETOHIhSYdqm4IGCpe85BOn5xRjgMFo"
    }


==================================================
==> 4. Login with wrong password should fail
==================================================
Request:
  Method: POST
  URL:    http://localhost:8080/api/v1/auth/login
  Token:  no
  Body:
    {
        "login":  "reader_auto_1774680823",
        "password":  "wrong_password"
    }

Response:
  Status: 401
  Body:
    {
        "error":  "invalid login or password"
    }


==================================================
==> 5. Create second user through /api/v1/users
==================================================
Request:
  Method: POST
  URL:    http://localhost:8080/api/v1/users
  Token:  no
  Body:
    {
        "login":  "reader2_auto_1774680823",
        "password":  "pass456",
        "firstName":  "Anna",
        "lastName":  "Smirnova"
    }

Response:
  Status: 201
  Body:
    {
        "firstName":  "Anna",
        "id":  2,
        "lastName":  "Smirnova",
        "login":  "reader2_auto_1774680823"
    }


==================================================
==> 6. Find user by login
==================================================
Request:
  Method: GET
  URL:    http://localhost:8080/api/v1/users/by-login/reader2_auto_1774680823
  Token:  no
  Body: <empty>

Response:
  Status: 200
  Body:
    {
        "firstName":  "Anna",
        "id":  2,
        "lastName":  "Smirnova",
        "login":  "reader2_auto_1774680823"
    }


==================================================
==> 7. Search users by first/last name
==================================================
Request:
  Method: GET
  URL:    http://localhost:8080/api/v1/users/search?firstName=Ann&lastName=Smir
  Token:  no
  Body: <empty>

Response:
  Status: 200
  Body:
    {
        "count":  1,
        "items":  [
                      {
                          "firstName":  "Anna",
                          "id":  2,
                          "lastName":  "Smirnova",
                          "login":  "reader2_auto_1774680823"
                      }
                  ]
    }


==================================================
==> 8. Create book without token should fail
==================================================
Request:
  Method: POST
  URL:    http://localhost:8080/api/v1/books
  Token:  no
  Body:
    {
        "title":  "War and Peace",
        "author":  "Leo Tolstoy",
        "year":  1869
    }

Response:
  Status: 401
  Body:
    {
        "error":  "missing or invalid bearer token"
    }


==================================================
==> 9. Create book with token
==================================================
Request:
  Method: POST
  URL:    http://localhost:8080/api/v1/books
  Token:  yes
  Body:
    {
        "title":  "War and Peace",
        "author":  "Leo Tolstoy",
        "year":  1869
    }

Response:
  Status: 201
  Body:
    {
        "author":  "Leo Tolstoy",
        "available":  true,
        "id":  1,
        "title":  "War and Peace",
        "year":  1869
    }


==================================================
==> 10. Search book by title
==================================================
Request:
  Method: GET
  URL:    http://localhost:8080/api/v1/books/search?title=War
  Token:  no
  Body: <empty>

Response:
  Status: 200
  Body:
    {
        "count":  1,
        "items":  [
                      {
                          "author":  "Leo Tolstoy",
                          "available":  true,
                          "id":  1,
                          "title":  "War and Peace",
                          "year":  1869
                      }
                  ]
    }


==================================================
==> 11. Search book by author
==================================================
Request:
  Method: GET
  URL:    http://localhost:8080/api/v1/books/search?author=Tolstoy
  Token:  no
  Body: <empty>

Response:
  Status: 200
  Body:
    {
        "count":  1,
        "items":  [
                      {
                          "author":  "Leo Tolstoy",
                          "available":  true,
                          "id":  1,
                          "title":  "War and Peace",
                          "year":  1869
                      }
                  ]
    }


==================================================
==> 12. Create loan without token should fail
==================================================
Request:
  Method: POST
  URL:    http://localhost:8080/api/v1/loans
  Token:  no
  Body:
    {
        "userId":  2,
        "bookId":  1,
        "issuedAt":  "2026-03-28"
    }

Response:
  Status: 401
  Body:
    {
        "error":  "missing or invalid bearer token"
    }


==================================================
==> 13. Create loan with token
==================================================
Request:
  Method: POST
  URL:    http://localhost:8080/api/v1/loans
  Token:  yes
  Body:
    {
        "userId":  2,
        "bookId":  1,
        "issuedAt":  "2026-03-28"
    }

Response:
  Status: 201
  Body:
    {
        "bookId":  1,
        "id":  1,
        "issuedAt":  "2026-03-28",
        "returned":  false,
        "returnedAt":  "",
        "userId":  2
    }


==================================================
==> 14. Create second loan for same book should fail
==================================================
Request:
  Method: POST
  URL:    http://localhost:8080/api/v1/loans
  Token:  yes
  Body:
    {
        "userId":  1,
        "bookId":  1,
        "issuedAt":  "2026-03-28"
    }

Response:
  Status: 409
  Body:
    {
        "error":  "book is already issued"
    }


==================================================
==> 15. Get loans of user
==================================================
Request:
  Method: GET
  URL:    http://localhost:8080/api/v1/users/2/loans
  Token:  no
  Body: <empty>

Response:
  Status: 200
  Body:
    {
        "count":  1,
        "items":  [
                      {
                          "bookId":  1,
                          "id":  1,
                          "issuedAt":  "2026-03-28",
                          "returned":  false,
                          "returnedAt":  "",
                          "userId":  2
                      }
                  ]
    }


==================================================
==> 16. Return loan without token should fail
==================================================
Request:
  Method: PATCH
  URL:    http://localhost:8080/api/v1/loans/1/return
  Token:  no
  Body:
    {
        "returnedAt":  "2026-04-05"
    }

Response:
  Status: 401
  Body:
    {
        "error":  "missing or invalid bearer token"
    }


==================================================
==> 17. Return loan with token
==================================================
Request:
  Method: PATCH
  URL:    http://localhost:8080/api/v1/loans/1/return
  Token:  yes
  Body:
    {
        "returnedAt":  "2026-04-05"
    }

Response:
  Status: 200
  Body:
    {
        "bookId":  1,
        "id":  1,
        "issuedAt":  "2026-03-28",
        "returned":  true,
        "returnedAt":  "2026-04-05",
        "userId":  2
    }


==================================================
==> 18. Return same loan again should fail
==================================================
Request:
  Method: PATCH
  URL:    http://localhost:8080/api/v1/loans/1/return
  Token:  yes
  Body:
    {
        "returnedAt":  "2026-04-05"
    }

Response:
  Status: 409
  Body:
    {
        "error":  "book already returned"
    }


==================================================
==> 19. User not found
==================================================
Request:
  Method: GET
  URL:    http://localhost:8080/api/v1/users/by-login/user_that_does_not_exist_12345
  Token:  no
  Body: <empty>

Response:
  Status: 404
  Body:
    {
        "error":  "user not found"
    }


==================================================
==> 20. Loan not found
==================================================
Request:
  Method: PATCH
  URL:    http://localhost:8080/api/v1/loans/999999/return
  Token:  yes
  Body:
    {
        "returnedAt":  "2026-04-05"
    }

Response:
  Status: 404
  Body:
    {
        "error":  "loan not found"
    }


==================================================
ALL TESTS PASSED
==================================================
```