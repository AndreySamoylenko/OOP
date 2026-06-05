#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <iomanip>
#include <string>
#include <limits>
#include <algorithm>

enum s_type
{
    STORAGE,
    DEBIT_CARD,
    CREDIT_CARD,
    DEPOSIT,
    CREDIT
};

struct state
{
    double money;
    double monthly_interest;
};

class Abstract_Storage
{
protected:
    double money;
    double monthly_interest;
    bool negative_handling;
    std::string name;
    s_type type = STORAGE;

public:
    Abstract_Storage() {}

    bool get_neg_handling() { return negative_handling; }

    s_type get_type() { return type; }

    std::string get_type_str()
    {
        switch (type)
        {
        case DEBIT_CARD:
            return "DebitCard";
        case CREDIT_CARD:
            return "CreditCard";
        case DEPOSIT:
            return "Deposit";
        case CREDIT:
            return "Credit";
        default:
            return "Storage";
        }
    }

    bool withdraw(double amount)
    {
        if (!negative_handling && amount > money)
            return false;
        money -= amount;
        return true;
    }
    void deposit(double amount) { money += amount; }

    double get_interest() const { return money; }
    bool set_interest(double i)
    {
        if (i < 0)
            return false;
        monthly_interest = i;
        return true;
    }
    virtual void process_interest() { money = money * (1 + monthly_interest); }

    double get_money() const { return money; }
    std::string get_name() const { return name; }

    state get_state() { return state{money, monthly_interest}; }
    void set_state(state s)
    {
        money = s.money;
        monthly_interest = s.monthly_interest;
    }
};

class DebitCard : public Abstract_Storage
{
public:
    DebitCard(std::string n)
    {
        name = n;
        money = 0;
        monthly_interest = 0;
        negative_handling = false;
        type = DEBIT_CARD;
    }
};

class CreditCard : public Abstract_Storage
{
public:
    CreditCard(std::string n)
    {
        name = n;
        money = 0;
        monthly_interest = 0;
        negative_handling = true;
        type = CREDIT_CARD;
    }
    void process_interest() override
    {
        if (money < 0)
            money = money * (1 + monthly_interest);
    }
};

class Deposit : public Abstract_Storage
{
public:
    Deposit(std::string n)
    {
        name = n;
        money = 0;
        monthly_interest = 0;
        negative_handling = false;
        type = DEPOSIT;
    }
};

class Credit : public Abstract_Storage
{
public:
    Credit(std::string n)
    {
        name = n;
        money = 0;
        monthly_interest = 0;
        negative_handling = true;
        type = CREDIT;
    }
    void process_interest() override
    {
        if (money < 0)
            money = money * (1 + monthly_interest);
    }
};

class Snapshot
{
    std::vector<state> history;
    int current_state = 0;

public:
    Snapshot(Abstract_Storage *src)
    {
        history.push_back(src->get_state());
    }

    void add_state(Abstract_Storage *src)
    {
        while ((int)history.size() - 1 > current_state)
            history.pop_back();
        history.push_back(src->get_state());
        current_state = (int)history.size() - 1;
    }

    state get_last_state() const { return history.back(); }
    state get_state_at(int index) const
    {
        if (index >= 0 && index < (int)history.size())
            return history.at(index);
        return state{0, 0};
    }

    int size() const { return (int)history.size(); }
    int get_current() const { return current_state; }

    bool undo(Abstract_Storage *target)
    {
        if (current_state <= 0)
            return false;
        current_state--;
        target->set_state(history[current_state]);
        return true;
    }
    bool redo(Abstract_Storage *target)
    {
        if (current_state >= (int)history.size() - 1)
            return false;
        current_state++;
        target->set_state(history[current_state]);
        return true;
    }

    bool goto_state(int index, Abstract_Storage *target)
    {
        if (index < 0 || index >= (int)history.size())
            return false;
        current_state = index;
        target->set_state(history[index]);
        return true;
    }
};

struct StorageObject
{
    Abstract_Storage *storage;
    Snapshot *snapshot;

    StorageObject(Abstract_Storage *s) : storage(s), snapshot(new Snapshot(s)) {}
    ~StorageObject()
    {
        delete storage;
        delete snapshot;
    }
};

class Account
{
private:
    std::string name;
    double cash;
    std::vector<StorageObject *> storages;

    std::vector<double> cash_history;
    int cash_state = 0;

    StorageObject *find_storage(const std::string &n)
    {
        for (auto strg : storages)
            if (strg->storage->get_name() == n)
                return strg;
        return nullptr;
    }

    Abstract_Storage *find(const std::string &n)
    {
        auto strg = find_storage(n);
        return strg ? strg->storage : nullptr;
    }

    void save_cash_snapshot()
    {
        while ((int)cash_history.size() - 1 > cash_state)
            cash_history.pop_back();
        cash_history.push_back(cash);
        cash_state = (int)cash_history.size() - 1;
    }

public:
    Account(std::string n) : name(n), cash(0)
    {
        cash_history.push_back(0);
    }

    ~Account()
    {
        for (auto strg : storages)
            delete strg;
    }

    std::string get_name() const { return name; }
    double get_cash() const { return cash; }

    // ----- adders -----
    bool add_debit_card(const std::string &n)
    {
        if (find(n))
            return false;
        storages.push_back(new StorageObject(new DebitCard(n)));
        return true;
    }
    bool add_credit_card(const std::string &n)
    {
        if (find(n))
            return false;
        storages.push_back(new StorageObject(new CreditCard(n)));
        return true;
    }
    bool add_deposit(const std::string &n)
    {
        if (find(n))
            return false;
        storages.push_back(new StorageObject(new Deposit(n)));
        return true;
    }
    bool add_credit(const std::string &n)
    {
        if (find(n))
            return false;
        storages.push_back(new StorageObject(new Credit(n)));
        return true;
    }

    bool remove_storage(const std::string &n)
    {
        for (auto it = storages.begin(); it != storages.end(); ++it)
        {
            if ((*it)->storage->get_name() == n)
            {
                delete *it;
                storages.erase(it);
                return true;
            }
        }
        return false;
    }

    // ----- cash operations -----
    bool spend_cash(double amount)
    {
        if (amount > cash)
            return false;
        cash -= amount;
        save_cash_snapshot();
        return true;
    }
    void receive_cash(double amount)
    {
        cash += amount;
        save_cash_snapshot();
    }

    // ----- transfers -----
    bool deposit_cash_to(const std::string &n, double amount)
    {
        StorageObject *strg = find_storage(n);
        if (amount > cash || !strg)
            return false;
        cash -= amount;
        strg->storage->deposit(amount);
        save_cash_snapshot();
        strg->snapshot->add_state(strg->storage);
        return true;
    }

    bool withdraw_from(const std::string &n, double amount)
    {
        StorageObject *strg = find_storage(n);
        if (!strg)
            return false;
        bool result = strg->storage->withdraw(amount);
        if (result)
        {
            cash += amount;
            save_cash_snapshot();
            strg->snapshot->add_state(strg->storage);
        }
        return result;
    }

    bool transfer(const std::string &from, const std::string &to, double amount)
    {
        StorageObject *ef = find_storage(from);
        StorageObject *et = find_storage(to);
        if (!ef || !et)
            return false;
        bool result = ef->storage->withdraw(amount);
        if (result)
        {
            et->storage->deposit(amount);
            ef->snapshot->add_state(ef->storage);
            et->snapshot->add_state(et->storage);
        }
        return result;
    }

    bool set_interest(const std::string &n, double i)
    {
        StorageObject *strg = find_storage(n);
        if (!strg)
            return false;
        bool r = strg->storage->set_interest(i);
        if (r)
            strg->snapshot->add_state(strg->storage);
        return r;
    }

    bool direct_deposit(const std::string &n, double amount)
    {
        StorageObject *strg = find_storage(n);
        if (!strg)
            return false;
        strg->storage->deposit(amount);
        strg->snapshot->add_state(strg->storage);
        return true;
    }

    bool direct_withdraw(const std::string &n, double amount)
    {
        StorageObject *strg = find_storage(n);
        if (!strg)
            return false;
        bool r = strg->storage->withdraw(amount);
        if (r)
            strg->snapshot->add_state(strg->storage);
        return r;
    }

    void monthly_interest_processing()
    {
        for (auto strg : storages)
        {
            strg->storage->process_interest();
            strg->snapshot->add_state(strg->storage);
        }
    }

    // ----- snapshot operations -----
    bool undo_storage(const std::string &n)
    {
        StorageObject *strg = find_storage(n);
        if (!strg)
            return false;
        return strg->snapshot->undo(strg->storage);
    }

    bool redo_storage(const std::string &n)
    {
        StorageObject *strg = find_storage(n);
        if (!strg)
            return false;
        return strg->snapshot->redo(strg->storage);
    }

    bool goto_storage_state(const std::string &n, int idx)
    {
        StorageObject *strg = find_storage(n);
        if (!strg)
            return false;
        return strg->snapshot->goto_state(idx, strg->storage);
    }

    bool undo_cash()
    {
        if (cash_state <= 0)
            return false;
        cash_state--;
        cash = cash_history[cash_state];
        return true;
    }

    bool redo_cash()
    {
        if (cash_state >= (int)cash_history.size() - 1)
            return false;
        cash_state++;
        cash = cash_history[cash_state];
        return true;
    }

    // ----- info -----
    std::vector<StorageObject *> &get_entries() { return storages; }

    StorageObject *get_entry(const std::string &n) { return find_storage(n); }

    double total_balance() const
    {
        double t = cash;
        for (auto strg : storages)
            t += strg->storage->get_money();
        return t;
    }
};

// ======================== UI ========================

std::string fmt(double v)
{
    std::ostringstream o;
    o << std::fixed << std::setprecision(2) << v;
    return o.str();
}

static void print_table(Account &acc)
{
    std::cout << "\n";
    std::cout << std::left
              << std::setw(20) << "Название"
              << std::setw(14) << "Тип"
              << std::setw(14) << "Баланс"
              << std::setw(12) << "%/мес" << "\n";
    std::cout << std::string(60, '-') << "\n";
    std::cout << std::left << std::setw(20) << "Наличные" << std::setw(14) << "-"
              << std::setw(14) << fmt(acc.get_cash()) << std::setw(12) << "-" << "\n";
    for (auto strg : acc.get_entries())
    {
        Abstract_Storage *s = strg->storage;
        std::cout << std::left
                  << std::setw(20) << s->get_name()
                  << std::setw(14) << s->get_type_str()
                  << std::setw(14) << fmt(s->get_money())
                  << std::setw(12) << fmt(strg->snapshot->get_last_state().monthly_interest * 100) + "%" << "\n";
    }
    std::cout << std::string(60, '-') << "\n";
    std::cout << "Итого: " << fmt(acc.total_balance()) << "\n\n";
}

static int read_int()
{
    int v;
    while (!(std::cin >> v))
    {
        std::cin.clear();
        std::cin.ignore(1000, '\n');
        std::cout << "Число: ";
    }
    std::cin.ignore(1000, '\n');
    return v;
}
static double read_double()
{
    double v;
    while (!(std::cin >> v))
    {
        std::cin.clear();
        std::cin.ignore(1000, '\n');
        std::cout << "Число: ";
    }
    std::cin.ignore(1000, '\n');
    return v;
}
static std::string read_str()
{
    std::string s;
    std::getline(std::cin, s);
    return s;
}

// ======================== MAIN ========================

int main()
{
    Account acc("Мой счёт");

    while (true)
    {
        print_table(acc);

        std::cout << "1. Наличные\n"
                  << "2. Операции с хранилищем\n"
                  << "3. Добавить хранилище\n"
                  << "4. Удалить хранилище\n"
                  << "5. Начислить проценты\n"
                  << "6. История версий\n"
                  << "0. Выход\n> ";
        int chosen_action = read_int();

        if (chosen_action == 0)
            break;

        std::string n, n2;
        double a;
        int idx;

        if (chosen_action == 1)
        {
            std::cout << "1.Получить  2.Потратить  3.На карту  4.С карты  5.Undo  6.Redo\n> ";
            int choice = read_int();
            if (choice == 1)
            {
                std::cout << "Сумма: ";
                a = read_double();
                acc.receive_cash(a);
            }
            else if (choice == 2)
            {
                std::cout << "Сумма: ";
                a = read_double();
                if (!acc.spend_cash(a))
                    std::cout << "Ошибка\n";
            }
            else if (choice == 3)
            {
                std::cout << "Карта: ";
                n = read_str();
                std::cout << "Сумма: ";
                a = read_double();
                if (!acc.deposit_cash_to(n, a))
                    std::cout << "Ошибка\n";
            }
            else if (choice == 4)
            {
                std::cout << "Карта: ";
                n = read_str();
                std::cout << "Сумма: ";
                a = read_double();
                if (!acc.withdraw_from(n, a))
                    std::cout << "Ошибка\n";
            }
            else if (choice == 5)
            {
                if (!acc.undo_cash())
                    std::cout << "Нечего отменять\n";
            }
            else if (choice == 6)
            {
                if (!acc.redo_cash())
                    std::cout << "Нечего повторять\n";
            }
        }
        else if (chosen_action == 2)
        {
            std::cout << "Хранилище: ";
            n = read_str();
            std::cout << "1.Пополнить  2.Снять  3.Перевод  4.Процент\n> ";
            int choice = read_int();
            if (choice == 1)
            {
                std::cout << "Сумма: ";
                a = read_double();
                if (!acc.direct_deposit(n, a))
                    std::cout << "Ошибка\n";
            }
            else if (choice == 2)
            {
                std::cout << "Сумма: ";
                a = read_double();
                if (!acc.direct_withdraw(n, a))
                    std::cout << "Ошибка\n";
            }
            else if (choice == 3)
            {
                std::cout << "Куда: ";
                n2 = read_str();
                std::cout << "Сумма: ";
                a = read_double();
                if (!acc.transfer(n, n2, a))
                    std::cout << "Ошибка\n";
            }
            else if (choice == 4)
            {
                std::cout << "Процент (0.05=5%): ";
                a = read_double();
                if (!acc.set_interest(n, a))
                    std::cout << "Ошибка\n";
            }
        }
        else if (chosen_action == 3)
        {
            std::cout << "1.Дебет  2.Кредитка  3.Вклад  4.Кредит\n> ";
            int choice = read_int();
            std::cout << "Название: ";
            n = read_str();
            bool ok = false;
            if (choice == 1)
                ok = acc.add_debit_card(n);
            else if (choice == 2)
                ok = acc.add_credit_card(n);
            else if (choice == 3)
                ok = acc.add_deposit(n);
            else if (choice == 4)
                ok = acc.add_credit(n);
            if (!ok)
                std::cout << "Ошибка (имя занято?)\n";
        }
        else if (chosen_action == 4)
        {
            std::cout << "Название: ";
            n = read_str();
            if (!acc.remove_storage(n))
                std::cout << "Не найдено\n";
        }
        else if (chosen_action == 5)
        {
            acc.monthly_interest_processing();
            std::cout << "Готово\n";
        }
        else if (chosen_action == 6)
        {
            std::cout << "Хранилище: ";
            n = read_str();
            StorageObject *strg = acc.get_entry(n);
            if (!strg)
            {
                std::cout << "Не найдено\n";
                continue;
            }
            Snapshot *sn = strg->snapshot;

            std::cout << "\nВерсии: " << n << "\n";
            std::cout << std::left << std::setw(6) << "#" << std::setw(14) << "Баланс" << std::setw(12) << "%/мес" << "Статус\n";
            std::cout << std::string(40, '-') << "\n";
            for (int i = 0; i < sn->size(); i++)
            {
                state s = sn->get_state_at(i);
                std::cout << std::left
                          << std::setw(6) << i
                          << std::setw(14) << fmt(s.money)
                          << std::setw(12) << fmt(s.monthly_interest * 100) + "%"
                          << (i == sn->get_current() ? "<-- текущая" : "") << "\n";
            }
            std::cout << "\n1.Перейти к версии  2.Undo  3.Redo\n> ";
            int choice = read_int();
            if (choice == 1)
            {
                std::cout << "Номер: ";
                idx = read_int();
                if (!acc.goto_storage_state(n, idx))
                    std::cout << "Ошибка\n";
            }
            else if (choice == 2)
            {
                if (!acc.undo_storage(n))
                    std::cout << "Нечего отменять\n";
            }
            else if (choice == 3)
            {
                if (!acc.redo_storage(n))
                    std::cout << "Нечего повторять\n";
            }
        }
    }
}
