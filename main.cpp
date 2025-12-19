#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <libpq-fe.h>
#include <iomanip>
#include <ctime>

struct Product { int id; std::string name, category; double price; };
struct Customer { int id; std::string name, region; };
struct Sale { int id, product_id, customer_id, quantity; double amount; std::string sale_date_str; };

std::string toDate(const std::string& date_str) {
    std::tm tm = {};
    std::istringstream ss(date_str);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    if (ss.fail()) return "";
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

std::vector<Product> loadProducts(const std::string& file) {
    std::vector<Product> data;
    std::ifstream f(file);
    if (!f.is_open()) return data;
    std::string line; std::getline(f, line); // header
    while (std::getline(f, line)) {
        std::stringstream ss(line);
        Product p; std::string temp;
        std::getline(ss, temp, ','); p.id = std::stoi(temp);
        std::getline(ss, p.name, ',');
        std::getline(ss, p.category, ',');
        std::getline(ss, temp, ','); p.price = std::stod(temp);
        data.push_back(p);
    }
    return data;
}

std::vector<Customer> loadCustomers(const std::string& file) {
    std::vector<Customer> data;
    std::ifstream f(file);
    if (!f.is_open()) return data;
    std::string line; std::getline(f, line);
    while (std::getline(f, line)) {
        std::stringstream ss(line);
        Customer c; std::string temp;
        std::getline(ss, temp, ','); c.id = std::stoi(temp);
        std::getline(ss, c.name, ',');
        std::getline(ss, c.region, ',');
        data.push_back(c);
    }
    return data;
}

std::vector<Sale> loadSales(const std::string& file) {
    std::vector<Sale> data;
    std::ifstream f(file);
    if (!f.is_open()) return data;
    std::string line; std::getline(f, line);
    while (std::getline(f, line)) {
        std::stringstream ss(line);
        Sale s; std::string temp;
        std::getline(ss, temp, ','); s.id = std::stoi(temp);
        std::getline(ss, s.sale_date_str, ',');
        std::getline(ss, temp, ','); s.product_id = std::stoi(temp);
        std::getline(ss, temp, ','); s.customer_id = std::stoi(temp);
        std::getline(ss, temp, ','); s.quantity = std::stoi(temp);
        std::getline(ss, temp, ','); s.amount = std::stod(temp);
        data.push_back(s);
    }
    return data;
}

bool exec(PGconn* conn, const std::string& sql) {
    PGresult* res = PQexec(conn, sql.c_str());
    bool ok = PQresultStatus(res) == PGRES_COMMAND_OK;
    if (!ok) std::cerr << "SQL Error: " << PQerrorMessage(conn) << std::endl;
    PQclear(res);
    return ok;
}

int main() {
    // Замените на свой пароль!
    const char* conninfo = "host=localhost port=5432 dbname=my_db user=postgres password=mypassword123";

    PGconn* conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Connection failed: " << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        return 1;
    }
    std::cout << "✅ Connected to my_db!" << std::endl;

    // ETL: Extract
    auto products = loadProducts("products.csv");
    auto customers = loadCustomers("customers.csv");
    auto sales = loadSales("sales.csv");

    std::cout << "📊 Loaded: " << products.size() << " products, "
              << customers.size() << " customers, " << sales.size() << " sales" << std::endl;

    // Load Dimensions (T)
    for (const auto& p : products) {
        std::string sql = "INSERT INTO products_dim VALUES (" +
                         std::to_string(p.id) + ", '" + p.name + "', '" +
                         p.category + "', " + std::to_string(p.price) +
                         ") ON CONFLICT DO NOTHING";
        exec(conn, sql);
    }

    for (const auto& c : customers) {
        std::string sql = "INSERT INTO customers_dim VALUES (" +
                         std::to_string(c.id) + ", '" + c.name + "', '" +
                         c.region + "') ON CONFLICT DO NOTHING";
        exec(conn, sql);
    }

    // Load Facts (T + L)
    for (const auto& s : sales) {
        std::string date = toDate(s.sale_date_str);
        if (date.empty()) continue;

        std::string sql = "INSERT INTO sales_fact (sale_id, sale_date, product_id, "
                         "customer_id, quantity, amount) VALUES (" +
                         std::to_string(s.id) + ", '" + date + "', " +
                         std::to_string(s.product_id) + ", " +
                         std::to_string(s.customer_id) + ", " +
                         std::to_string(s.quantity) + ", " +
                         std::to_string(s.amount) + ")";
        exec(conn, sql);
    }

    std::cout << "✅ Data loaded!" << std::endl;

    // Analytics
    std::cout << "\n📈 ANALYTICS:\n";

    PGresult* res = PQexec(conn,
        "SELECT p.category, SUM(s.amount) total_sales, COUNT(s.sale_id) cnt "
        "FROM sales_fact s JOIN products_dim p ON s.product_id = p.product_id "
        "GROUP BY p.category ORDER BY total_sales DESC");

    for (int i = 0; i < PQntuples(res); ++i) {
        std::cout << PQgetvalue(res, i, 0) << ": " << PQgetvalue(res, i, 1)
                  << " (n=" << PQgetvalue(res, i, 2) << ")" << std::endl;
    }
    PQclear(res);

    PQfinish(conn);
    return 0;
}
