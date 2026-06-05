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
        if ((int)history.size() - 1 != current_state)
        {
            while ((int)history.size() - 1 > current_state)
                history.pop_back();
        }
        history.push_back(src->get_state());
        current_state++;
    }

    state get_last_state() const { return history.back(); }

    state get_state_at(int index)
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

    // View without applying
    state peek_state(int index)
    {
        if (index < 0 || index >= (int)history.size())
            return state{0, 0};
        return history[index];
    }
};

struct StorageEntry
{
    Abstract_Storage *storage;
    Snapshot *snapshot;

    StorageEntry(Abstract_Storage *s) : storage(s), snapshot(new Snapshot(s)) {}
    ~StorageEntry()
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
    std::vector<StorageEntry *> entries;

    // Snapshot for cash
    std::vector<double> cash_history;
    int cash_state = 0;

    StorageEntry *find_entry(const std::string &n)
    {
        for (auto e : entries)
            if (e->storage->get_name() == n)
                return e;
        return nullptr;
    }

    Abstract_Storage *find(const std::string &n)
    {
        auto e = find_entry(n);
        return e ? e->storage : nullptr;
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
        for (auto e : entries)
            delete e;
    }

    std::string get_name() const { return name; }
    double get_cash() const { return cash; }

    // ----- adders -----
    bool add_debit_card(const std::string &n)
    {
        if (find(n))
            return false;
        entries.push_back(new StorageEntry(new DebitCard(n)));
        return true;
    }
    bool add_credit_card(const std::string &n)
    {
        if (find(n))
            return false;
        entries.push_back(new StorageEntry(new CreditCard(n)));
        return true;
    }
    bool add_deposit(const std::string &n)
    {
        if (find(n))
            return false;
        entries.push_back(new StorageEntry(new Deposit(n)));
        return true;
    }
    bool add_credit(const std::string &n)
    {
        if (find(n))
            return false;
        entries.push_back(new StorageEntry(new Credit(n)));
        return true;
    }

    bool remove_storage(const std::string &n)
    {
        for (auto it = entries.begin(); it != entries.end(); ++it)
        {
            if ((*it)->storage->get_name() == n)
            {
                delete *it;
                entries.erase(it);
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
        StorageEntry *e = find_entry(n);
        if (amount > cash || !e)
            return false;
        cash -= amount;
        e->storage->deposit(amount);
        save_cash_snapshot();
        e->snapshot->add_state(e->storage);
        return true;
    }

    bool withdraw_from(const std::string &n, double amount)
    {
        StorageEntry *e = find_entry(n);
        if (!e)
            return false;
        bool result = e->storage->withdraw(amount);
        if (result)
        {
            cash += amount;
            save_cash_snapshot();
            e->snapshot->add_state(e->storage);
        }
        return result;
    }

    bool transfer(const std::string &from, const std::string &to, double amount)
    {
        StorageEntry *ef = find_entry(from);
        StorageEntry *et = find_entry(to);
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
        StorageEntry *e = find_entry(n);
        if (!e)
            return false;
        bool r = e->storage->set_interest(i);
        if (r)
            e->snapshot->add_state(e->storage);
        return r;
    }

    bool direct_deposit(const std::string &n, double amount)
    {
        StorageEntry *e = find_entry(n);
        if (!e)
            return false;
        e->storage->deposit(amount);
        e->snapshot->add_state(e->storage);
        return true;
    }

    bool direct_withdraw(const std::string &n, double amount)
    {
        StorageEntry *e = find_entry(n);
        if (!e)
            return false;
        bool r = e->storage->withdraw(amount);
        if (r)
            e->snapshot->add_state(e->storage);
        return r;
    }

    void monthly_interest_processing()
    {
        for (auto e : entries)
        {
            e->storage->process_interest();
            e->snapshot->add_state(e->storage);
        }
    }

    // ----- snapshot operations -----
    bool undo_storage(const std::string &n)
    {
        StorageEntry *e = find_entry(n);
        if (!e)
            return false;
        return e->snapshot->undo(e->storage);
    }

    bool redo_storage(const std::string &n)
    {
        StorageEntry *e = find_entry(n);
        if (!e)
            return false;
        return e->snapshot->redo(e->storage);
    }

    bool goto_storage_state(const std::string &n, int idx)
    {
        StorageEntry *e = find_entry(n);
        if (!e)
            return false;
        return e->snapshot->goto_state(idx, e->storage);
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
    std::vector<StorageEntry *> &get_entries() { return entries; }

    StorageEntry *get_entry(const std::string &n) { return find_entry(n); }

    double total_balance() const
    {
        double t = cash;
        for (auto e : entries)
            t += e->storage->get_money();
        return t;
    }
};

// hehhhhhhhhhhhhhh

// ======================== UI HELPERS ========================

const std::string RESET = "\033[0m";
const std::string BOLD = "\033[1m";
const std::string DIM = "\033[2m";
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string CYAN = "\033[36m";
const std::string MAGENTA = "\033[35m";
const std::string WHITE = "\033[97m";
const std::string BG_DARK = "\033[40m";

void clear_screen()
{
    system("cls");
}

std::string money_str(double m)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << m;
    return oss.str();
}

std::string colored_money(double m)
{
    if (m < 0)
        return RED + money_str(m) + RESET;
    if (m > 0)
        return GREEN + money_str(m) + RESET;
    return DIM + "0.00" + RESET;
}

void print_separator(int width = 70)
{
    std::cout << DIM;
    for (int i = 0; i < width; i++)
        std::cout << "?";
    std::cout << RESET << "\n";
}

void print_header(const std::string &title)
{
    int w = 70;
    print_separator(w);
    int pad = (w - (int)title.size()) / 2;
    std::cout << BOLD << CYAN;
    for (int i = 0; i < pad; i++)
        std::cout << " ";
    std::cout << title << RESET << "\n";
    print_separator(w);
}

void print_table_header(const std::vector<std::string> &cols, const std::vector<int> &widths)
{
    std::cout << BOLD << YELLOW;
    for (int i = 0; i < (int)cols.size(); i++)
        std::cout << std::left << std::setw(widths[i]) << cols[i];
    std::cout << RESET << "\n";
    print_separator(70);
}

double read_double(const std::string &prompt)
{
    double v;
    while (true)
    {
        std::cout << CYAN << prompt << RESET;
        if (std::cin >> v)
        {
            std::cin.ignore();
            return v;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << RED << "  Неверный ввод. Введите число.\n"
                  << RESET;
    }
}

std::string read_string(const std::string &prompt)
{
    std::string s;
    std::cout << CYAN << prompt << RESET;
    std::getline(std::cin, s);
    return s;
}

int read_int(const std::string &prompt)
{
    int v;
    while (true)
    {
        std::cout << CYAN << prompt << RESET;
        if (std::cin >> v)
        {
            std::cin.ignore();
            return v;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << RED << "  Неверный ввод. Введите целое число.\n"
                  << RESET;
    }
}

void ok(const std::string &msg) { std::cout << GREEN << "  ? " << msg << RESET << "\n"; }
void err(const std::string &msg) { std::cout << RED << "  ? " << msg << RESET << "\n"; }
void info(const std::string &msg) { std::cout << YELLOW << "  ? " << msg << RESET << "\n"; }

void press_enter()
{
    std::cout << DIM << "\n  [Enter для продолжения]" << RESET;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// ======================== MAIN MENU SECTIONS ========================

void show_overview(Account &acc)
{
    print_header("  ОБЗОР СЧЁТА: " + acc.get_name() + "  ");

    std::vector<std::string> cols = {"Название", "Тип", "Баланс (руб)", "Процент/мес"};
    std::vector<int> widths = {20, 14, 16, 12};
    print_table_header(cols, widths);

    // Cash row
    std::cout << std::left
              << std::setw(20) << "Наличные"
              << std::setw(14) << "Cash"
              << std::setw(16) << colored_money(acc.get_cash())
              << std::setw(12) << "—"
              << "\n";

    for (auto e : acc.get_entries())
    {
        Abstract_Storage *s = e->storage;
        double interest = e->snapshot->get_last_state().monthly_interest * 100.0;
        std::string interest_str = money_str(interest) + "%";

        std::cout << std::left
                  << std::setw(20) << s->get_name()
                  << std::setw(14) << s->get_type_str()
                  << std::setw(16) << colored_money(s->get_money())
                  << std::setw(12) << interest_str
                  << "\n";
    }

    print_separator();
    std::cout << BOLD << "  Итого: " << colored_money(acc.total_balance()) << RESET << "\n\n";
}

void menu_add_storage(Account &acc)
{
    print_header("  ДОБАВИТЬ ХРАНИЛИЩЕ  ");
    std::cout << "  1. Дебетовая карта\n";
    std::cout << "  2. Кредитная карта\n";
    std::cout << "  3. Вклад (Deposit)\n";
    std::cout << "  4. Кредит\n";
    std::cout << "  0. Назад\n\n";

    int ch = read_int("  Выбор: ");
    if (ch == 0)
        return;

    std::string n = read_string("  Название: ");
    if (n.empty())
    {
        err("Имя не может быть пустым.");
        press_enter();
        return;
    }

    bool r = false;
    switch (ch)
    {
    case 1:
        r = acc.add_debit_card(n);
        break;
    case 2:
        r = acc.add_credit_card(n);
        break;
    case 3:
        r = acc.add_deposit(n);
        break;
    case 4:
        r = acc.add_credit(n);
        break;
    default:
        err("Неверный выбор.");
        press_enter();
        return;
    }
    r ? ok("Добавлено: " + n) : err("Имя уже существует: " + n);
    press_enter();
}

void menu_cash(Account &acc)
{
    while (true)
    {
        print_header("  НАЛИЧНЫЕ  ");
        std::cout << "  Текущий баланс: " << colored_money(acc.get_cash()) << "\n\n";
        std::cout << "  1. Получить наличные (доход)\n";
        std::cout << "  2. Потратить наличные (расход)\n";
        std::cout << "  3. Внести наличные на карту/вклад\n";
        std::cout << "  4. Снять с карты/вклада в наличные\n";
        std::cout << "  5. Отменить последнее действие (undo)\n";
        std::cout << "  6. Повторить (redo)\n";
        std::cout << "  0. Назад\n\n";

        int ch = read_int("  Выбор: ");
        if (ch == 0)
            break;

        double amount;
        std::string name;

        switch (ch)
        {
        case 1:
            amount = read_double("  Сумма: ");
            acc.receive_cash(amount);
            ok("Получено: " + money_str(amount) + " руб.");
            break;
        case 2:
            amount = read_double("  Сумма: ");
            acc.spend_cash(amount) ? ok("Потрачено: " + money_str(amount)) : err("Недостаточно наличных.");
            break;
        case 3:
            name = read_string("  Название карты/вклада: ");
            amount = read_double("  Сумма: ");
            acc.deposit_cash_to(name, amount) ? ok("Переведено на " + name) : err("Ошибка: карта не найдена или недостаточно наличных.");
            break;
        case 4:
            name = read_string("  Название карты/вклада: ");
            amount = read_double("  Сумма: ");
            acc.withdraw_from(name, amount) ? ok("Снято с " + name) : err("Ошибка: карта не найдена или недостаточно средств.");
            break;
        case 5:
            acc.undo_cash() ? ok("Отменено.") : err("Нечего отменять.");
            break;
        case 6:
            acc.redo_cash() ? ok("Повторено.") : err("Нечего повторять.");
            break;
        default:
            err("Неверный выбор.");
        }
        press_enter();
    }
}

void menu_storage_ops(Account &acc)
{
    while (true)
    {
        print_header("  ОПЕРАЦИИ С КАРТАМИ / ВКЛАДАМИ  ");

        if (acc.get_entries().empty())
        {
            info("Нет хранилищ. Добавьте карту или вклад.");
            press_enter();
            return;
        }

        show_overview(acc);

        std::cout << "  1. Пополнить карту/вклад (без наличных)\n";
        std::cout << "  2. Снять с карты/вклада (без наличных)\n";
        std::cout << "  3. Перевод между картами\n";
        std::cout << "  4. Установить процентную ставку\n";
        std::cout << "  5. Начислить проценты (конец месяца)\n";
        std::cout << "  6. Удалить хранилище\n";
        std::cout << "  0. Назад\n\n";

        int ch = read_int("  Выбор: ");
        if (ch == 0)
            break;

        std::string name, name2;
        double amount;

        switch (ch)
        {
        case 1:
            name = read_string("  Название: ");
            amount = read_double("  Сумма: ");
            acc.direct_deposit(name, amount) ? ok("Пополнено.") : err("Карта/вклад не найдены.");
            break;
        case 2:
            name = read_string("  Название: ");
            amount = read_double("  Сумма: ");
            acc.direct_withdraw(name, amount) ? ok("Снято.") : err("Ошибка: не найдено или недостаточно средств.");
            break;
        case 3:
            name = read_string("  Откуда: ");
            name2 = read_string("  Куда: ");
            amount = read_double("  Сумма: ");
            acc.transfer(name, name2, amount) ? ok("Перевод выполнен.") : err("Ошибка перевода.");
            break;
        case 4:
            name = read_string("  Название: ");
            amount = read_double("  Процент в месяц (0.05 = 5%): ");
            acc.set_interest(name, amount) ? ok("Ставка установлена.") : err("Ошибка: карта не найдена или неверное значение.");
            break;
        case 5:
            acc.monthly_interest_processing();
            ok("Проценты начислены.");
            break;
        case 6:
            name = read_string("  Название: ");
            acc.remove_storage(name) ? ok("Удалено: " + name) : err("Не найдено: " + name);
            break;
        default:
            err("Неверный выбор.");
        }
        press_enter();
    }
}

void menu_history(Account &acc)
{
    while (true)
    {
        print_header("  ИСТОРИЯ ВЕРСИЙ  ");

        if (acc.get_entries().empty())
        {
            info("Нет хранилищ.");
            press_enter();
            return;
        }

        // Show list
        std::cout << "  Хранилища:\n";
        for (auto e : acc.get_entries())
        {
            Snapshot *sn = e->snapshot;
            std::cout << "    " << BOLD << e->storage->get_name() << RESET
                      << " [" << e->storage->get_type_str() << "]"
                      << "  версий: " << sn->size()
                      << "  текущая: " << sn->get_current() << "\n";
        }
        std::cout << "\n";

        std::cout << "  1. Просмотреть версии хранилища\n";
        std::cout << "  2. Перейти к версии (применить)\n";
        std::cout << "  3. Отменить (undo)\n";
        std::cout << "  4. Повторить (redo)\n";
        std::cout << "  0. Назад\n\n";

        int ch = read_int("  Выбор: ");
        if (ch == 0)
            break;

        std::string name;

        switch (ch)
        {
        case 1:
        {
            name = read_string("  Название хранилища: ");
            StorageEntry *e = acc.get_entry(name);
            if (!e)
            {
                err("Не найдено.");
                break;
            }

            Snapshot *sn = e->snapshot;
            print_separator();
            std::cout << BOLD << "  Версии: " << name << RESET << "\n";
            print_separator();

            std::vector<std::string> cols = {"#", "Баланс", "Процент/мес", "Статус"};
            std::vector<int> widths = {6, 18, 16, 12};
            print_table_header(cols, widths);

            for (int i = 0; i < sn->size(); i++)
            {
                state s = sn->peek_state(i);
                std::string status = (i == sn->get_current()) ? (BOLD + GREEN + "? текущая" + RESET) : "";
                std::cout << std::left
                          << std::setw(6) << i
                          << std::setw(18) << colored_money(s.money)
                          << std::setw(16) << (money_str(s.monthly_interest * 100) + "%")
                          << status << "\n";
            }
            print_separator();
            break;
        }
        case 2:
        {
            name = read_string("  Название хранилища: ");
            int idx = read_int("  Номер версии: ");
            acc.goto_storage_state(name, idx) ? ok("Восстановлено до версии " + std::to_string(idx)) : err("Ошибка.");
            break;
        }
        case 3:
            name = read_string("  Название хранилища: ");
            acc.undo_storage(name) ? ok("Отменено.") : err("Нечего отменять.");
            break;
        case 4:
            name = read_string("  Название хранилища: ");
            acc.redo_storage(name) ? ok("Повторено.") : err("Нечего повторять.");
            break;
        default:
            err("Неверный выбор.");
        }
        press_enter();
    }
}

// ======================== MAIN ========================

int main()
{
    // Set UTF-8 output on Windows
#ifdef _WIN32
    system("chcp 65001 > nul");
#endif

    clear_screen();
    print_header("  УЧЁТ ДОХОДОВ И РАСХОДОВ  ");
    std::cout << "\n";

    std::string acc_name = read_string("  Имя владельца счёта: ");
    if (acc_name.empty())
        acc_name = "Мой счёт";

    Account acc(acc_name);

    // Pre-populate with example data
    acc.add_debit_card("Сбербанк");
    acc.add_credit_card("Тинькофф");
    acc.add_deposit("Вклад ВТБ");
    acc.receive_cash(5000);

    while (true)
    {
        clear_screen();
        show_overview(acc);

        print_header("  ГЛАВНОЕ МЕНЮ  ");
        std::cout << "  1. Наличные\n";
        std::cout << "  2. Операции с картами / вкладами\n";
        std::cout << "  3. Добавить карту / вклад / кредит\n";
        std::cout << "  4. История версий (Snapshot)\n";
        std::cout << "  0. Выход\n\n";

        int ch = read_int("  Выбор: ");

        switch (ch)
        {
        case 0:
            clear_screen();
            std::cout << BOLD << CYAN << "\n  До свидания!\n\n"
                      << RESET;
            return 0;
        case 1:
            clear_screen();
            menu_cash(acc);
            break;
        case 2:
            clear_screen();
            menu_storage_ops(acc);
            break;
        case 3:
            clear_screen();
            menu_add_storage(acc);
            break;
        case 4:
            clear_screen();
            menu_history(acc);
            break;
        default:
            err("Неверный выбор.");
            press_enter();
            break;
        }
    }
}