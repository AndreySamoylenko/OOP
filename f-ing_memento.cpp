#include <iostream>
#include <sstream>
#include <map>
#include <vector>

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

// ======================== BASECLASS ===========================
class Abstract_Storage
{
protected:
    double money;
    double monthly_interest; // 0.1 = 10% начислений от суммы в мес€ц (дл€ кредитки обрабатываетс€ отдельно)
    bool negative_handling;
    std::string name;
    s_type type = STORAGE;

public:
    Abstract_Storage() {}

    bool get_neg_handling() { return negative_handling; }

    s_type get_type()
    {
        return type;
    }
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
    void deposit(double amount)
    {
        money += amount;
    }

    double get_interest() const { return money; }
    bool set_interest(double i)
    {
        if (i < 0)
            return false;
        monthly_interest = i;
        return true;
    }
    void process_interest()
    {
        money = money * (1 + monthly_interest);
    }

    double get_money() const { return money; }
    std::string get_name() const { return name; }

    state get_state()
    {
        return state{money, monthly_interest};
    }
};

// =============== DESCENDERS ===============

class DebitCard : public Abstract_Storage
{
public:
    DebitCard(std::string n)
    {
        name = n;
        money = 0;
        monthly_interest = 0;
        negative_handling = false;
        s_type type = DEBIT_CARD;
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
        s_type type = CREDIT_CARD;
    };

    void process_interest()
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
        s_type type = DEPOSIT;
    };
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
        s_type type = CREDIT;
    };

    void process_interest()
    {
        if (money < 0)
            money = money * (1 + monthly_interest);
    }

    // возможно надо ещЄ типа деструктор вызывать когда сумма положительна€ (кредит закрыт) но пока плевать
};

class Snapshot : public Abstract_Storage
{
    std::vector<state> history;
    int current_state = 0;

public:
    Snapshot(Abstract_Storage *src)
    {
        history.push_back(src->get_state());
    }

    ~Snapshot() { delete &history; }

    void add_state(Abstract_Storage *src)
    {
        if (history.size() - 1 != current_state)
        { // € проще словами объ€ню чем в комменте
            for (int i = current_state + 1; i < history.size(); i++)
            {
                history.pop_back();
            }
        }
        history.push_back(src->get_state());
        current_state++;
    }

    state get_last_state() const
    {
        return history.back();
    }

    
};

class Account
{
private:
    std::string name;
    double cash;
    std::vector<Abstract_Storage> storages;

    Abstract_Storage *find(std::string n)
    {
        for (auto it = storages.begin(); it != storages.end(); ++it)
        {
            if (it.base()->get_name() == n)
            {
                return it.base();
            }
        }
        return nullptr;
    }

public:
    Account(std::string n)
    {
        name = n;
        cash = 0;
    }

    // ----- adders -----

    void add_debit_card(std::string n) { storages.push_back(DebitCard(n)); }
    void add_credit_card(std::string n) { storages.push_back(CreditCard(n)); }
    void add_deposit(std::string n) { storages.push_back(Deposit(n)); }
    void add_credit(std::string n) { storages.push_back(Credit(n)); }

    // ----- cash operations -----

    // decrease cash amount
    bool spend_cash(double amount)
    {
        if (amount > cash)
            return false;
        cash -= amount;
        return true;
    }
    // increase cash amount
    void recieve_cash(double amount) { cash += amount; }

    // ----- card and deposit operations -----

    // decrease cash and add that amount to some card or deposit
    bool deposit_cash_to(std::string n, double amount)
    {
        Abstract_Storage *target = find(n);
        if (amount > cash || !target)
            return false;

        cash -= amount;
        target->deposit(amount);
        return true;
    }
    // decrease card or deposit money and add that amount to cash
    bool withdraw_from(std::string n, double amount)
    {
        Abstract_Storage *target = find(n);
        if (!target)
            return false;

        bool result = target->withdraw(amount);
        cash += result * amount;

        return result;
    }
    void monthly_interest_processing()
    {
        for (auto it = storages.begin(); it != storages.end(); ++it)
        {
            it.base()->process_interest();
        }
    }

    Abstract_Storage *get_storage(std::string n)
    {
        return find(n);
    }
};