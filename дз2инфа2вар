
CREATE TABLE IF NOT EXISTS products_dim (
    product_id SERIAL PRIMARY KEY,
    product_name VARCHAR(255) NOT NULL,
    category VARCHAR(100),
    price DECIMAL(10,2)
);

CREATE TABLE IF NOT EXISTS customers_dim (
    customer_id SERIAL PRIMARY KEY,
    customer_name VARCHAR(255) NOT NULL,
    region VARCHAR(100)
);

CREATE TABLE IF NOT EXISTS time_dim (
    time_id SERIAL PRIMARY KEY,
    date DATE UNIQUE NOT NULL,
    year INT, quarter INT, month INT, day INT,
    day_of_week VARCHAR(20)
);

CREATE TABLE IF NOT EXISTS sales_fact (
    sale_id SERIAL PRIMARY KEY,
    sale_date DATE NOT NULL,
    product_id INT REFERENCES products_dim(product_id),
    customer_id INT REFERENCES customers_dim(customer_id),
    quantity INT NOT NULL,
    amount DECIMAL(10,2) NOT NULL
);

-- Индексы (тоже безопасно)
DO $$ BEGIN
    CREATE INDEX CONCURRENTLY IF NOT EXISTS idx_sales_product ON sales_fact(product_id);
EXCEPTION WHEN others THEN NULL; END $$;

DO $$ BEGIN
    CREATE INDEX CONCURRENTLY IF NOT EXISTS idx_sales_customer ON sales_fact(customer_id);
EXCEPTION WHEN others THEN NULL; END $$;

DO $$ BEGIN
    CREATE INDEX CONCURRENTLY IF NOT EXISTS idx_sales_date ON sales_fact(sale_date);
EXCEPTION WHEN others THEN NULL; END $$;
