CREATE EXTENSION IF NOT EXISTS pg_trgm;

DROP TABLE IF EXISTS loans CASCADE;

DROP TABLE IF EXISTS books CASCADE;

DROP TABLE IF EXISTS users CASCADE;

CREATE TABLE users (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    login VARCHAR(100) NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,
    first_name VARCHAR(100) NOT NULL,
    last_name VARCHAR(100) NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    CONSTRAINT chk_users_login_not_blank CHECK (btrim(login) <> ''),
    CONSTRAINT chk_users_first_name_not_blank CHECK (btrim(first_name) <> ''),
    CONSTRAINT chk_users_last_name_not_blank CHECK (btrim(last_name) <> '')
);

CREATE TABLE books (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    title VARCHAR(255) NOT NULL,
    author VARCHAR(255) NOT NULL,
    publication_year INTEGER NOT NULL,
    available BOOLEAN NOT NULL DEFAULT TRUE,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    CONSTRAINT chk_books_title_not_blank CHECK (btrim(title) <> ''),
    CONSTRAINT chk_books_author_not_blank CHECK (btrim(author) <> ''),
    CONSTRAINT chk_books_publication_year CHECK (
        publication_year BETWEEN 1450
        AND EXTRACT(
            YEAR
            FROM
                NOW()
        ) :: INT + 1
    )
);

CREATE TABLE loans (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    user_id BIGINT NOT NULL,
    book_id BIGINT NOT NULL,
    issued_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    returned_at TIMESTAMPTZ NULL,
    returned BOOLEAN NOT NULL DEFAULT FALSE,
    CONSTRAINT fk_loans_user FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE RESTRICT,
    CONSTRAINT fk_loans_book FOREIGN KEY (book_id) REFERENCES books(id) ON DELETE RESTRICT,
    CONSTRAINT chk_loans_return_state CHECK (
        (
            returned = FALSE
            AND returned_at IS NULL
        )
        OR (
            returned = TRUE
            AND returned_at IS NOT NULL
        )
    ),
    CONSTRAINT chk_loans_returned_after_issue CHECK (
        returned_at IS NULL
        OR returned_at >= issued_at
    )
);

CREATE INDEX idx_loans_user_id ON loans(user_id);

CREATE INDEX idx_loans_book_id ON loans(book_id);

CREATE INDEX idx_loans_user_issued_at ON loans(user_id, issued_at DESC);

CREATE INDEX idx_loans_active_book ON loans(book_id)
WHERE
    returned = FALSE;

CREATE UNIQUE INDEX uq_loans_book_active ON loans(book_id)
WHERE
    returned = FALSE;

CREATE INDEX idx_users_first_name_trgm ON users USING gin (lower(first_name) gin_trgm_ops);

CREATE INDEX idx_users_last_name_trgm ON users USING gin (lower(last_name) gin_trgm_ops);

CREATE INDEX idx_books_title_trgm ON books USING gin (lower(title) gin_trgm_ops);

CREATE INDEX idx_books_author_trgm ON books USING gin (lower(author) gin_trgm_ops);

COMMENT ON TABLE users IS 'Пользователи библиотеки и учетные записи для авторизации';

COMMENT ON TABLE books IS 'Книги, доступные для выдачи';

COMMENT ON TABLE loans IS 'Факты выдачи и возврата книг';