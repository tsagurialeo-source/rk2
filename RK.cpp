//============================================================================
// Name        : RK.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <string>
#include <iomanip>

using namespace std;

// Базовый класс: банковский счет
class BankAccount {
protected:
    string accountNumber;  // номер счета
    string ownerName;      // имя владельца
    double balance;        // баланс

public:
    // Конструктор
    BankAccount(string accNum, string name, double initialBalance = 0.0)
        : accountNumber(accNum), ownerName(name), balance(initialBalance) {
        if (initialBalance < 0) {
            balance = 0.0;
            cout << "Начальный баланс не может быть отрицательным. Установлен в 0.0" << endl;
        }
    }

    // Метод для пополнения средств
    void deposit(double amount) {
        if (amount > 0) {
            balance += amount;
            cout << "Успешно пополнено: " << amount << " руб." << endl;
        } else {
            cout << "Ошибка: сумма пополнения должна быть положительной." << endl;
        }
    }

    // Метод для снятия средств
    bool withdraw(double amount) {
        if (amount > 0 && amount <= balance) {
            balance -= amount;
            cout << "Успешно снято: " << amount << " руб." << endl;
            return true;
        } else if (amount > balance) {
            cout << "Ошибка: недостаточно средств на счете." << endl;
            return false;
        } else {
            cout << "Ошибка: сумма снятия должна быть положительной." << endl;
            return false;
        }
    }

    // Метод для получения баланса
    double getBalance() const {
        return balance;
    }

    // Метод для отображения информации о счете
    virtual void displayInfo() const {
        cout << "\n=== Информация о банковском счете ===" << endl;
        cout << "Номер счета: " << accountNumber << endl;
        cout << "Владелец: " << ownerName << endl;
        cout << "Баланс: " << fixed << setprecision(2) << balance << " руб." << endl;
    }

    // Виртуальный деструктор
    virtual ~BankAccount() {}
};

// Производный класс: сберегательный счет
class SavingsAccount : public BankAccount {
private:
    double interestRate;  // процентная ставка (в процентах)

public:
    // Конструктор
    SavingsAccount(string accNum, string name, double initialBalance = 0.0, double rate = 0.0)
        : BankAccount(accNum, name, initialBalance), interestRate(rate) {
        if (rate < 0) {
            interestRate = 0.0;
            cout << "Процентная ставка не может быть отрицательной. Установлена в 0.0%" << endl;
        }
    }

    // Метод для начисления процентов
    void applyInterest() {
        double interest = balance * (interestRate / 100.0);
        balance += interest;
        cout << "Начислены проценты: " << fixed << setprecision(2)
             << interest << " руб. (ставка: " << interestRate << "%)" << endl;
    }

    // Метод для получения процентной ставки
    double getInterestRate() const {
        return interestRate;
    }

    // Метод для изменения процентной ставки
    void setInterestRate(double rate) {
        if (rate >= 0) {
            interestRate = rate;
            cout << "Процентная ставка изменена на: " << rate << "%" << endl;
        } else {
            cout << "Ошибка: процентная ставка не может быть отрицательной." << endl;
        }
    }

    // Переопределенный метод для отображения информации
    void displayInfo() const override {
        cout << "\n=== Информация о сберегательном счете ===" << endl;
        cout << "Номер счета: " << accountNumber << endl;
        cout << "Владелец: " << ownerName << endl;
        cout << "Баланс: " << fixed << setprecision(2) << balance << " руб." << endl;
        cout << "Процентная ставка: " << interestRate << "%" << endl;
    }
};

int main() {
    cout << "=== Моделирование работы банка ===\n" << endl;

    // Создание обычного банковского счета
    cout << "1. Создание обычного банковского счета:" << endl;
    BankAccount regularAccount("RU1234567890", "Иван Иванов", 5000.0);
    regularAccount.displayInfo();

    // Операции с обычным счетом
    cout << "\n2. Операции с обычным счетом:" << endl;
    regularAccount.deposit(1500.0);
    cout << "Баланс после пополнения: " << regularAccount.getBalance() << " руб." << endl;

    regularAccount.withdraw(2000.0);
    cout << "Баланс после снятия: " << regularAccount.getBalance() << " руб." << endl;

    // Попытка снять больше, чем есть на счете
    regularAccount.withdraw(6000.0);

    // Создание сберегательного счета
    cout << "\n3. Создание сберегательного счета:" << endl;
    SavingsAccount savingsAccount("RU0987654321", "Петр Петров", 10000.0, 5.0);
    savingsAccount.displayInfo();

    // Операции со сберегательным счетом
    cout << "\n4. Операции со сберегательным счетом:" << endl;
    savingsAccount.deposit(3000.0);
    cout << "Баланс после пополнения: " << savingsAccount.getBalance() << " руб." << endl;

    savingsAccount.withdraw(1500.0);
    cout << "Баланс после снятия: " << savingsAccount.getBalance() << " руб." << endl;

    // Начисление процентов
    cout << "\n5. Начисление процентов на сберегательный счет:" << endl;
    savingsAccount.applyInterest();
    savingsAccount.displayInfo();

    // Начисление процентов еще раз
    cout << "\n6. Еще одно начисление процентов:" << endl;
    savingsAccount.applyInterest();
    savingsAccount.displayInfo();

    // Изменение процентной ставки
    cout << "\n7. Изменение процентной ставки:" << endl;
    savingsAccount.setInterestRate(7.5);
    savingsAccount.applyInterest();
    savingsAccount.displayInfo();

    // Демонстрация полиморфизма
    cout << "\n8. Демонстрация полиморфизма:" << endl;
    BankAccount* accounts[2];

    accounts[0] = new BankAccount("RU1111111111", "Алексей Сидоров", 3000.0);
    accounts[1] = new SavingsAccount("RU2222222222", "Мария Васильева", 8000.0, 4.0);

    for (int i = 0; i < 2; i++) {
        accounts[i]->displayInfo();
        cout << endl;
    }

    // Очистка памяти
    for (int i = 0; i < 2; i++) {
        delete accounts[i];
    }

    return 0;
}
