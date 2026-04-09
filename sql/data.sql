INSERT INTO
    users (
        login,
        password_hash,
        first_name,
        last_name,
        created_at
    )
VALUES
    (
        'reader01',
        'secret123',
        'Ivan',
        'Petrov',
        '2026-03-01 10:00:00+00'
    ),
    (
        'reader02',
        'secret123',
        'Anna',
        'Smirnova',
        '2026-03-01 10:05:00+00'
    ),
    (
        'reader03',
        'secret123',
        'Pavel',
        'Ivanov',
        '2026-03-01 10:10:00+00'
    ),
    (
        'reader04',
        'secret123',
        'Elena',
        'Sokolova',
        '2026-03-01 10:15:00+00'
    ),
    (
        'reader05',
        'secret123',
        'Maria',
        'Kuznetsova',
        '2026-03-01 10:20:00+00'
    ),
    (
        'reader06',
        'secret123',
        'Dmitry',
        'Volkov',
        '2026-03-01 10:25:00+00'
    ),
    (
        'reader07',
        'secret123',
        'Olga',
        'Popova',
        '2026-03-01 10:30:00+00'
    ),
    (
        'reader08',
        'secret123',
        'Sergey',
        'Fedorov',
        '2026-03-01 10:35:00+00'
    ),
    (
        'reader09',
        'secret123',
        'Natalia',
        'Morozova',
        '2026-03-01 10:40:00+00'
    ),
    (
        'reader10',
        'secret123',
        'Alexey',
        'Lebedev',
        '2026-03-01 10:45:00+00'
    );

INSERT INTO
    books (
        title,
        author,
        publication_year,
        available,
        created_at
    )
VALUES
    (
        'War and Peace',
        'Leo Tolstoy',
        1869,
        FALSE,
        '2026-03-01 11:00:00+00'
    ),
    (
        'Crime and Punishment',
        'Fyodor Dostoevsky',
        1866,
        TRUE,
        '2026-03-01 11:05:00+00'
    ),
    (
        'The Master and Margarita',
        'Mikhail Bulgakov',
        1967,
        FALSE,
        '2026-03-01 11:10:00+00'
    ),
    (
        'Fathers and Sons',
        'Ivan Turgenev',
        1862,
        TRUE,
        '2026-03-01 11:15:00+00'
    ),
    (
        'Anna Karenina',
        'Leo Tolstoy',
        1877,
        FALSE,
        '2026-03-01 11:20:00+00'
    ),
    (
        'The Idiot',
        'Fyodor Dostoevsky',
        1869,
        TRUE,
        '2026-03-01 11:25:00+00'
    ),
    (
        'Dead Souls',
        'Nikolai Gogol',
        1842,
        FALSE,
        '2026-03-01 11:30:00+00'
    ),
    (
        'We',
        'Yevgeny Zamyatin',
        1924,
        TRUE,
        '2026-03-01 11:35:00+00'
    ),
    (
        'Heart of a Dog',
        'Mikhail Bulgakov',
        1925,
        FALSE,
        '2026-03-01 11:40:00+00'
    ),
    (
        'Doctor Zhivago',
        'Boris Pasternak',
        1957,
        TRUE,
        '2026-03-01 11:45:00+00'
    );

INSERT INTO
    loans (
        user_id,
        book_id,
        issued_at,
        returned_at,
        returned
    )
VALUES
    (1, 1, '2026-03-05 09:00:00+00', NULL, FALSE),
    (2, 3, '2026-03-05 10:00:00+00', NULL, FALSE),
    (3, 5, '2026-03-06 08:30:00+00', NULL, FALSE),
    (4, 7, '2026-03-06 12:15:00+00', NULL, FALSE),
    (5, 9, '2026-03-07 14:20:00+00', NULL, FALSE),
    (
        6,
        2,
        '2026-03-02 09:00:00+00',
        '2026-03-10 16:00:00+00',
        TRUE
    ),
    (
        7,
        4,
        '2026-03-02 11:00:00+00',
        '2026-03-11 13:30:00+00',
        TRUE
    ),
    (
        8,
        6,
        '2026-03-03 15:00:00+00',
        '2026-03-12 10:10:00+00',
        TRUE
    ),
    (
        9,
        8,
        '2026-03-04 17:00:00+00',
        '2026-03-13 09:45:00+00',
        TRUE
    ),
    (
        10,
        10,
        '2026-03-04 18:00:00+00',
        '2026-03-14 11:20:00+00',
        TRUE
    );