-- 1. Создание таблиц
CREATE TABLE students (
    student_id SERIAL PRIMARY KEY,
    full_name VARCHAR(100) NOT NULL,
    group_number VARCHAR(20) NOT NULL
);

CREATE TABLE subjects (
    subject_id SERIAL PRIMARY KEY,
    subject_name VARCHAR(100) NOT NULL UNIQUE
);

CREATE TABLE grades (
    grade_id SERIAL PRIMARY KEY,
    student_id INTEGER REFERENCES students(student_id) ON DELETE CASCADE,
    subject_id INTEGER REFERENCES subjects(subject_id) ON DELETE CASCADE,
    grade INTEGER CHECK (grade >= 2 AND grade <= 5) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE attendance (
    attendance_id SERIAL PRIMARY KEY,
    student_id INTEGER REFERENCES students(student_id) ON DELETE CASCADE,
    date_attended DATE NOT NULL,
    status VARCHAR(10) CHECK (status IN ('present', 'absent', 'late')) NOT NULL
);

CREATE TABLE notes (
    note_id SERIAL PRIMARY KEY,
    student_id INTEGER REFERENCES students(student_id) ON DELETE CASCADE,
    note_text TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 2. Наполнение данных

-- Добавление 6 студентов одной группы
INSERT INTO students (full_name, group_number) VALUES
('Иванов Иван Иванович', 'Группа 101'),
('Петров Петр Петрович', 'Группа 101'),
('Сидорова Анна Сергеевна', 'Группа 101'),
('Кузнецова Елена Владимировна', 'Группа 101'),
('Смирнов Алексей Дмитриевич', 'Группа 101'),
('Попова Мария Андреевна', 'Группа 101');

-- Добавление 3 предметов
INSERT INTO subjects (subject_name) VALUES
('Математический анализ'),
('Аналитическая геометрия'),
('Информатика');

-- Добавление оценок для всех студентов по каждому предмету
-- Для каждого студента добавляем по 2-3 оценки по каждому предмету
INSERT INTO grades (student_id, subject_id, grade) VALUES
-- Студент 1 (Иванов)
(1, 1, 5), (1, 1, 4), (1, 2, 5), (1, 2, 5), (1, 3, 4), (1, 3, 5),
-- Студент 2 (Петров)
(2, 1, 3), (2, 1, 4), (2, 2, 3), (2, 2, 4), (2, 3, 5), (2, 3, 5),
-- Студент 3 (Сидорова)
(3, 1, 5), (3, 1, 5), (3, 2, 4), (3, 2, 5), (3, 3, 5), (3, 3, 5),
-- Студент 4 (Кузнецова)
(4, 1, 4), (4, 1, 4), (4, 2, 4), (4, 2, 3), (4, 3, 4), (4, 3, 5),
-- Студент 5 (Смирнов)
(5, 1, 3), (5, 1, 4), (5, 2, 5), (5, 2, 4), (5, 3, 3), (5, 3, 4),
-- Студент 6 (Попова)
(6, 1, 5), (6, 1, 5), (6, 2, 5), (6, 2, 5), (6, 3, 5), (6, 3, 4);

-- Добавление записей посещаемости для всех студентов на два дня
INSERT INTO attendance (student_id, date_attended, status) VALUES
-- День 1 (2024-03-01)
(1, '2024-03-01', 'present'),
(2, '2024-03-01', 'present'),
(3, '2024-03-01', 'present'),
(4, '2024-03-01', 'late'),
(5, '2024-03-01', 'absent'),
(6, '2024-03-01', 'present'),
-- День 2 (2024-03-02)
(1, '2024-03-02', 'present'),
(2, '2024-03-02', 'present'),
(3, '2024-03-02', 'present'),
(4, '2024-03-02', 'present'),
(5, '2024-03-02', 'late'),
(6, '2024-03-02', 'present');

-- Добавление заметок по каждому студенту
INSERT INTO notes (student_id, note_text) VALUES
(1, 'Любит информатику, всегда активно участвует на занятиях'),
(2, 'Нужна помощь по информатике, особенно в программировании'),
(3, 'Отличник по всем предметам, особенно сильна в математическом анализе'),
(4, 'Хорошо работает в команде, прогресс по информатике заметен'),
(5, 'Редко посещает занятия по информатике, нужна дополнительная мотивация'),
(6, 'Прогресс по информатике значительный, стала лучше понимать алгоритмы');

-- 3. Индексы

-- Индекс на group_number в таблице students для быстрого поиска одногруппников
CREATE INDEX idx_students_group_number ON students(group_number);

-- Индекс на student_id в таблице grades для ускорения агрегированных запросов
CREATE INDEX idx_grades_student_id ON grades(student_id);

-- Для полнотекстового поиска в PostgreSQL
-- Сначала добавляем колонку для поиска (если еще нет)
ALTER TABLE notes ADD COLUMN search_vector tsvector;

-- Обновляем tsvector колонку
UPDATE notes SET search_vector = to_tsvector('russian', note_text);

-- Создаем GIN индекс для полнотекстового поиска
CREATE INDEX idx_notes_search ON notes USING GIN(search_vector);

-- Создаем триггер для автоматического обновления search_vector
CREATE OR REPLACE FUNCTION notes_search_vector_update() RETURNS trigger AS $$
BEGIN
    NEW.search_vector = to_tsvector('russian', NEW.note_text);
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER tsvector_update BEFORE INSERT OR UPDATE ON notes
FOR EACH ROW EXECUTE FUNCTION notes_search_vector_update();

-- 4. Представления

-- Создание представления для вычисления среднего балла каждого студента по всем предметам
CREATE VIEW student_avg_grades AS
SELECT 
    s.student_id,
    s.full_name,
    s.group_number,
    ROUND(AVG(g.grade)::numeric, 2) as average_grade,
    COUNT(g.grade_id) as total_grades
FROM students s
LEFT JOIN grades g ON s.student_id = g.student_id
GROUP BY s.student_id, s.full_name, s.group_number;

-- 5. Транзакция для добавления нового студента с оценками

-- Транзакция для добавления нового студента вместе с его оценками
BEGIN;

-- Добавляем нового студента
INSERT INTO students (full_name, group_number) 
VALUES ('Новиков Дмитрий Сергеевич', 'Группа 101')
RETURNING student_id;

-- Сохраняем ID нового студента (в реальном приложении это можно сделать через переменную)
-- Для примера будем использовать student_id = 7
INSERT INTO grades (student_id, subject_id, grade) VALUES
(7, 1, 4), -- Математический анализ
(7, 2, 5), -- Аналитическая геометрия
(7, 3, 4); -- Информатика

COMMIT;

-- 6. Запросы

-- Запрос 1: Найти 5 одногруппников студента (2 до и 3 после него по списку student_id)
-- Для студента с student_id = 3
WITH student_group AS (
    SELECT group_number 
    FROM students 
    WHERE student_id = 3
)
SELECT 
    s.student_id,
    s.full_name,
    s.group_number
FROM students s
CROSS JOIN student_group sg
WHERE s.group_number = sg.group_number
  AND s.student_id != 3
ORDER BY s.student_id
LIMIT 5;

-- Альтернативный вариант: 2 до и 3 после конкретного студента
(SELECT * FROM students 
 WHERE student_id < 3 AND group_number = (SELECT group_number FROM students WHERE student_id = 3)
 ORDER BY student_id DESC LIMIT 2)
UNION ALL
(SELECT * FROM students 
 WHERE student_id > 3 AND group_number = (SELECT group_number FROM students WHERE student_id = 3)
 ORDER BY student_id ASC LIMIT 3)
ORDER BY student_id;

-- Запрос 2: Посмотреть средний балл конкретного студента через представление
SELECT * FROM student_avg_grades WHERE student_id = 3;

-- Запрос 3: Агрегировать средний балл по предмету «Информатика»
SELECT 
    s.subject_name,
    ROUND(AVG(g.grade)::numeric, 2) as average_grade,
    COUNT(g.grade_id) as total_grades
FROM subjects s
JOIN grades g ON s.subject_id = g.subject_id
WHERE s.subject_name = 'Информатика'
GROUP BY s.subject_id, s.subject_name;

-- Запрос 4: Выполнить полнотекстовый поиск по заметкам, содержащим слово «Информатика»
SELECT 
    n.note_id,
    s.full_name,
    n.note_text,
    n.created_at
FROM notes n
JOIN students s ON n.student_id = s.student_id
WHERE n.search_vector @@ to_tsquery('russian', 'Информатика')
ORDER BY n.created_at DESC;

-- Запрос 5: Обновить посещаемость студента на конкретную дату через транзакцию
BEGIN;

-- Проверяем существующую запись
SELECT * FROM attendance 
WHERE student_id = 5 AND date_attended = '2024-03-01';

-- Обновляем статус посещаемости
UPDATE attendance 
SET status = 'present'
WHERE student_id = 5 AND date_attended = '2024-03-01';

-- Если записи нет, создаем новую
INSERT INTO attendance (student_id, date_attended, status)
SELECT 5, '2024-03-01', 'present'
WHERE NOT EXISTS (
    SELECT 1 FROM attendance 
    WHERE student_id = 5 AND date_attended = '2024-03-01'
);

COMMIT;

-- Дополнительные полезные запросы:

-- Показать всех студентов с их средними баллами
SELECT * FROM student_avg_grades ORDER BY average_grade DESC;

-- Показать посещаемость по дням
SELECT 
    date_attended,
    COUNT(*) as total_students,
    COUNT(CASE WHEN status = 'present' THEN 1 END) as present,
    COUNT(CASE WHEN status = 'absent' THEN 1 END) as absent,
    COUNT(CASE WHEN status = 'late' THEN 1 END) as late
FROM attendance
GROUP BY date_attended
ORDER BY date_attended;

-- Показать успеваемость по предметам
SELECT 
    s.subject_name,
    ROUND(AVG(g.grade)::numeric, 2) as average_grade,
    MIN(g.grade) as min_grade,
    MAX(g.grade) as max_grade,
    COUNT(g.grade_id) as total_grades
FROM subjects s
JOIN grades g ON s.subject_id = g.subject_id
GROUP BY s.subject_id, s.subject_name
ORDER BY average_grade DESC;
