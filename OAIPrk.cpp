#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <memory>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iomanip>

using namespace std;

// Базовый класс ElectronicDevice
class ElectronicDevice {
protected:
    string brand;
    string model;
    double price;
    vector<string> installedApps; // Композиция: список установленных приложений

public:
    ElectronicDevice(const string& b, const string& m, double p, const vector<string>& apps)
        : brand(b), model(m), price(p), installedApps(apps) {}

    virtual ~ElectronicDevice() = default;

    // Виртуальная функция отображения
    virtual void display() const {
        cout << "Brand: " << brand << ", Model: " << model
             << ", Price: $" << fixed << setprecision(2) << price
             << ", Apps: ";
        for (const auto& app : installedApps) cout << app << " ";
        cout << endl;
    }

    // Функции для работы с файлами
    virtual void saveToFile(ofstream& out) const {
        out << brand << ";" << model << ";" << price << ";";
        for (size_t i = 0; i < installedApps.size(); ++i) {
            out << installedApps[i];
            if (i != installedApps.size() - 1) out << "|";
        }
    }

    virtual void loadFromFile(ifstream& in) {
        string appsStr;
        getline(in, brand, ';');
        getline(in, model, ';');
        in >> price;
        in.ignore(); // пропуск ';'
        getline(in, appsStr);

        installedApps.clear();
        stringstream ss(appsStr);
        string app;
        while (getline(ss, app, '|')) {
            installedApps.push_back(app);
        }
    }

    // Геттеры
    string getBrand() const { return brand; }
    string getModel() const { return model; }
    double getPrice() const { return price; }
    vector<string> getInstalledApps() const { return installedApps; }

    // Сеттеры
    void setPrice(double p) { price = p; }
    void setModel(const string& m) { model = m; }
    void addApp(const string& app) { installedApps.push_back(app); }
};

// Производный класс Smartphone
class Smartphone : public ElectronicDevice {
private:
    string os;
    int memory; // в ГБ

public:
    Smartphone(const string& b, const string& m, double p, const vector<string>& apps,
               const string& o, int mem)
        : ElectronicDevice(b, m, p, apps), os(o), memory(mem) {}

    void display() const override {
        cout << "[Smartphone] ";
        ElectronicDevice::display();
        cout << "  OS: " << os << ", Memory: " << memory << "GB" << endl;
    }

    void saveToFile(ofstream& out) const override {
        out << "Smartphone;";
        ElectronicDevice::saveToFile(out);
        out << ";" << os << "-" << memory << endl;
    }

    string getOS() const { return os; }
    int getMemory() const { return memory; }
};

// Производный класс Laptop
class Laptop : public ElectronicDevice {
private:
    double screenSize; // в дюймах
    int batteryCapacity; // в Вт·ч

public:
    Laptop(const string& b, const string& m, double p, const vector<string>& apps,
           double screen, int battery)
        : ElectronicDevice(b, m, p, apps), screenSize(screen), batteryCapacity(battery) {}

    void display() const override {
        cout << "[Laptop] ";
        ElectronicDevice::display();
        cout << "  Screen: " << screenSize << "\", Battery: " << batteryCapacity << "Wh" << endl;
    }

    void saveToFile(ofstream& out) const override {
        out << "Laptop;";
        ElectronicDevice::saveToFile(out);
        out << ";" << screenSize << "-" << batteryCapacity << endl;
    }

    double getScreenSize() const { return screenSize; }
    int getBatteryCapacity() const { return batteryCapacity; }
};

// Функция загрузки данных из файла
void loadFromFile(const string& filename, list<shared_ptr<ElectronicDevice>>& devices) {
    ifstream file(filename);
    if (!file) {
        cerr << "Cannot open file: " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string type, brand, model, param, appsStr;
        double price;

        getline(ss, type, ';');
        getline(ss, brand, ';');
        getline(ss, model, ';');
        ss >> price;
        ss.ignore();
        getline(ss, param, ';');
        getline(ss, appsStr);

        // Парсинг списка приложений
        vector<string> apps;
        stringstream appsStream(appsStr);
        string app;
        while (getline(appsStream, app, '|')) {
            apps.push_back(app);
        }

        if (type == "Smartphone") {
            string os;
            int memory;
            stringstream paramStream(param);
            getline(paramStream, os, '-');
            paramStream >> memory;
            devices.push_back(make_shared<Smartphone>(brand, model, price, apps, os, memory));
        } else if (type == "Laptop") {
            double screen;
            int battery;
            stringstream paramStream(param);
            paramStream >> screen;
            paramStream.ignore();
            paramStream >> battery;
            devices.push_back(make_shared<Laptop>(brand, model, price, apps, screen, battery));
        }
    }
    cout << "Data loaded from " << filename << endl;
}

// Функция сохранения данных в файл
void saveToFile(const string& filename, const list<shared_ptr<ElectronicDevice>>& devices) {
    ofstream file(filename);
    if (!file) {
        cerr << "Cannot open file for writing: " << filename << endl;
        return;
    }

    for (const auto& device : devices) {
        device->saveToFile(file);
    }
    cout << "Data saved to " << filename << endl;
}

// Функция изменения объекта через указатель
void editDevice(ElectronicDevice* device) {
    if (!device) return;

    cout << "Editing device: ";
    device->display();

    double newPrice;
    string newModel, newApp;

    cout << "Enter new price: ";
    cin >> newPrice;
    device->setPrice(newPrice);

    cin.ignore();
    cout << "Enter new model: ";
    getline(cin, newModel);
    device->setModel(newModel);

    cout << "Enter new app to add: ";
    getline(cin, newApp);
    device->addApp(newApp);

    cout << "Device updated.\n";
}

// Основное меню
void menu(list<shared_ptr<ElectronicDevice>>& devices) {
    int choice;
    do {
        cout << "\n========== Electronic Device Manager ==========\n";
        cout << "1. Load data from file\n";
        cout << "2. Display all devices\n";
        cout << "3. Search/Filter devices\n";
        cout << "4. Sort devices\n";
        cout << "5. Add new device manually\n";
        cout << "6. Edit existing device\n";
        cout << "7. Delete device\n";
        cout << "8. Save data to file\n";
        cout << "9. Special functions (lambdas)\n";
        cout << "0. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1: {
                string filename;
                cout << "Enter filename: ";
                cin >> filename;
                loadFromFile(filename, devices);
                break;
            }
            case 2: {
                cout << "\n--- All Devices ---\n";
                for (const auto& device : devices) {
                    device->display();
                }
                break;
            }
            case 3: {
                int filterChoice;
                cout << "Filter by:\n1. Price above\n2. OS\n3. App name\n";
                cin >> filterChoice;
                if (filterChoice == 1) {
                    double minPrice;
                    cout << "Enter minimum price: ";
                    cin >> minPrice;
                    auto it = copy_if(devices.begin(), devices.end(),
                                      ostream_iterator<shared_ptr<ElectronicDevice>>(cout, "\n"),
                                      [minPrice](const shared_ptr<ElectronicDevice>& d) {
                                          return d->getPrice() > minPrice;
                                      });
                }
                break;
            }
            case 4: {
                cout << "Sort by:\n1. Price\n2. Brand then price\n";
                int sortChoice;
                cin >> sortChoice;
                if (sortChoice == 1) {
                    devices.sort([](const shared_ptr<ElectronicDevice>& a,
                                    const shared_ptr<ElectronicDevice>& b) {
                        return a->getPrice() < b->getPrice();
                    });
                } else if (sortChoice == 2) {
                    devices.sort([](const shared_ptr<ElectronicDevice>& a,
                                    const shared_ptr<ElectronicDevice>& b) {
                        if (a->getBrand() != b->getBrand())
                            return a->getBrand() < b->getBrand();
                        return a->getPrice() > b->getPrice();
                    });
                }
                cout << "Sorted.\n";
                break;
            }
            case 5: {
                cout << "Manual addition not fully implemented in this example.\n";
                break;
            }
            case 6: {
                int index = 0;
                cout << "Select device to edit (0-" << devices.size()-1 << "):\n";
                for (const auto& d : devices) {
                    cout << index++ << ": ";
                    d->display();
                }
                int idx;
                cin >> idx;
                if (idx >= 0 && idx < devices.size()) {
                    auto it = devices.begin();
                    advance(it, idx);
                    editDevice(it->get());
                }
                break;
            }
            case 7: {
                int index = 0;
                cout << "Select device to delete (0-" << devices.size()-1 << "):\n";
                for (const auto& d : devices) {
                    cout << index++ << ": ";
                    d->display();
                }
                int idx;
                cin >> idx;
                if (idx >= 0 && idx < devices.size()) {
                    auto it = devices.begin();
                    advance(it, idx);
                    devices.erase(it);
                    cout << "Device deleted.\n";
                }
                break;
            }
            case 8: {
                string filename;
                cout << "Enter filename to save: ";
                cin >> filename;
                saveToFile(filename, devices);
                break;
            }
            case 9: {
                cout << "\n--- Special Lambda Functions ---\n";

                // 1. Поиск смартфона с наибольшей памятью
                auto maxMemPhone = max_element(devices.begin(), devices.end(),
                    [](const shared_ptr<ElectronicDevice>& a, const shared_ptr<ElectronicDevice>& b) {
                        auto* sa = dynamic_cast<Smartphone*>(a.get());
                        auto* sb = dynamic_cast<Smartphone*>(b.get());
                        if (!sa) return true;
                        if (!sb) return false;
                        return sa->getMemory() < sb->getMemory();
                    });
                if (maxMemPhone != devices.end()) {
                    cout << "Smartphone with max memory:\n";
                    (*maxMemPhone)->display();
                }

                // 2. Подсчёт ноутбуков с экраном > 15 дюймов
                int countLaptops = count_if(devices.begin(), devices.end(),
                    [](const shared_ptr<ElectronicDevice>& d) {
                        auto* laptop = dynamic_cast<Laptop*>(d.get());
                        return laptop && laptop->getScreenSize() > 15.0;
                    });
                cout << "Laptops with screen > 15\": " << countLaptops << endl;

                // 3. Сортировка по цене
                devices.sort([](const auto& a, const auto& b) {
                    return a->getPrice() < b->getPrice();
                });
                cout << "Devices sorted by price.\n";

                // 4. Фильтр по цене
                double minPrice;
                cout << "Enter price threshold: ";
                cin >> minPrice;
                cout << "Devices above $" << minPrice << ":\n";
                for (const auto& d : devices) {
                    if (d->getPrice() > minPrice) d->display();
                }

                // 5. Поиск смартфона с максимальной памятью (альтернативный способ)
                shared_ptr<Smartphone> maxPhone;
                for (const auto& d : devices) {
                    auto* phone = dynamic_cast<Smartphone*>(d.get());
                    if (phone && (!maxPhone || phone->getMemory() > maxPhone->getMemory())) {
                        maxPhone = dynamic_pointer_cast<Smartphone>(d);
                    }
                }
                if (maxPhone) {
                    cout << "Max memory smartphone (alt method):\n";
                    maxPhone->display();
                }

                // 6. Сортировка по бренду и цене
                devices.sort([](const auto& a, const auto& b) {
                    if (a->getBrand() != b->getBrand())
                        return a->getBrand() < b->getBrand();
                    return a->getPrice() > b->getPrice();
                });
                cout << "Sorted by brand (A-Z) then price (high-low).\n";
                break;
            }
            case 0:
                cout << "Exiting...\n";
                break;
            default:
                cout << "Invalid choice.\n";
        }
    } while (choice != 0);
}

int main() {
    list<shared_ptr<ElectronicDevice>> devices;

    // Автозагрузка данных из файла (по умолчанию)
    loadFromFile("devices.txt", devices);

    menu(devices);

    // Автосохранение при выходе
    saveToFile("devices_saved.txt", devices);

    return 0;
}
