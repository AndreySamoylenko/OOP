// 15. Система бронирования билетов
// Создайте систему покупки билетов на мероприятия:
//  - Одиночка: BookingSystem, управляющий бронированием.
//  - Наблюдатель: Уведомления о доступных билетах.
//  - Команда: Бронирование, отмена, возврат.

#include <iostream>
#include <map>
#include <vector>
#include <string>

class Observer;

struct Date
{
    unsigned short int day;
    unsigned short int month;
    unsigned int year;
};

template <typename T>
bool find(const std::vector<T *> &vec, T *item)
{
    for (const auto &elem : vec)
    {
        if (elem == item)
            return true;
    }
    return false;
}

class Event
{
private:
    std::string name;
    Date date;
    bool status; // true - bookable, false - fully booked
    std::vector<Observer *> subscribers;
    std::vector<Observer *> bookers;

public:
    Event(const std::string &n, const Date &d, const bool s) : name(n), date(d), status(s) {}

    const Date &getDate() const { return date; }
    const std::vector<Observer *> &getObservers() const { return subscribers; }
    bool getStatus() const { return status; }

    void subscribe(Observer *o)
    {
        subscribers.push_back(o);
        o->update("you are successfully subscribed to " + name + " event");
    }
    void unsubscribe(Observer *o)
    {
        for (auto observers_it = subscribers.begin(); observers_it != subscribers.end(); ++observers_it)
        {
            if ((*observers_it.base())->getName() == (o)->getName())
            {
                subscribers.erase(observers_it);
                o->update("you are successfully unsubscribed from " + name + " event");
                return;
            }
        }
        std::cout << "!!! You are not subscribed to this event !!!\n";
    }

    void book(Observer *o)
    {
        bookers.push_back(o);
        if (!find(subscribers, o))
            subscribe(o);
        o->update("you have successfully booked a ticket for " + name + " event");
    }
    void unbook(Observer *o)
    {
        for (auto bookers_it = bookers.begin(); bookers_it != bookers.end(); ++bookers_it)
        {
            if ((*bookers_it.base())->getName() == (o)->getName())
            {
                bookers.erase(bookers_it);
                o->update("you have successfully unbooked a ticket for " + name + " event");
                return;
            }
        }
        std::cout << "!!! You have not booked a ticket for this event !!!\n";
    }

    void notifyObservers(const std::string &message)
    {
        for (Observer *o : subscribers)
        {
            o->update(message);
        }
    }
    void setDate(const Date &d)
    {
        if (date.day != d.day || date.month != d.month || date.year != d.year)
        {
            date = d;
            std::string message = "Event date changed to " + std::to_string(d.day) + "." + std::to_string(d.month) + "." + std::to_string(d.year);
            notifyObservers(message);
        }
    }

    void setStatus(bool s)
    {
        if (status != s)
        {
            status = s;
            std::string message = "Event status changed to " + std::string(s ? "available for booking" : "fully booked");
            notifyObservers(message);
        }
    }
    ~Event()
    {
        notifyObservers("Event " + name + " is canceled, all tickets are refunded");
    }
};

class BookingSystem
{
private:
    static BookingSystem *instance;
    std::map<std::string, Event> events;

    BookingSystem() {}

public:
    static BookingSystem &get_instance()
    {
        static BookingSystem instance;
        return instance;
    }

    void add_event(const std::string &name, const Date &date, const bool status = true)
    {
        Event ev(name, date, status);
        events[name] = ev;
    }

    void remove_event(const std::string &name)
    {
        events.erase(name);
    }

    Event *get_event(const std::string &name)
    {
        if (events.find(name) != events.end())
            return &events[name];
        return nullptr;
    }

    std::map<std::string, Event> &get_events()
    {
        return events;
    }
};

class Observer
{

public:
    virtual void update(const std::string &message) = 0;
    virtual std::string getName() const { return ""; }
    virtual ~Observer() {}
};

class Customer : public Observer
{
private:
    std::string name;

public:
    Customer(const std::string &name) : name(name) {}

    void update(const std::string &message) override
    {
        std::cout << name << " получил уведомление: " << message << std::endl;
    }

    std::string getName() const
    {
        return name;
    }
};

class Comand
{
public:
    virtual void execute() = 0;
    virtual ~Comand() {}
};

class BookTicket : public Comand
{
private:
    BookingSystem *bs;
    std::string e_name;
    Customer *customer;

public:
    BookTicket(BookingSystem *b, std::string e, Customer *c)
    {
        bs = b;
        e_name = e;
        customer = c;
    }

    void execute()
    {
        Event *ev = bs->get_event(e_name);
        if (ev && ev->getStatus())
        {
            ev->book(customer);
        }
        else
        {
            std::cout << "Event is inaccessible or nonexistent\n";
        }
    }
};

class RefundTicket : public Comand
{
private:
    BookingSystem *bs;
    std::string e_name;
    Customer *customer;

public:
    RefundTicket(BookingSystem *b, std::string e, Customer *c)
    {
        bs = b;
        e_name = e;
        customer = c;
    }

    void execute()
    {
        Event *ev = bs->get_event(e_name);
        if (ev && !ev->getObservers().empty())
        {
            ev->unbook(customer);
        }
        else
        {
            std::cout << "Event is inaccessible or has sold no tickets\n";
        }
    }
};

class Subscribe : public Comand
{
private:
    BookingSystem *bs;
    std::string e_name;
    Customer *customer;

public:
    Subscribe(BookingSystem *b, std::string e, Customer *c)
    {
        bs = b;
        e_name = e;
        customer = c;
    }

    void execute()
    {
        Event *ev = bs->get_event(e_name);
        if (ev && ev->getStatus())
        {
            ev->subscribe(customer);
        }
        else
        {
            std::cout << "Event is inaccessible or nonexistent\n";
        }
    }
};

class Unsubscribe : public Comand
{
private:
    BookingSystem *bs;
    std::string e_name;
    Customer *customer;

public:
    Unsubscribe(BookingSystem *b, std::string e, Customer *c)
    {
        bs = b;
        e_name = e;
        customer = c;
    }

    void execute()
    {
        Event *ev = bs->get_event(e_name);
        if (ev && !ev->getObservers().empty())
        {
            ev->unsubscribe(customer);
        }
        else
        {
            std::cout << "Event is inaccessible or has no subscribers\n";
        }
    }
};

class CancelEvent : public Comand
{
private:
    BookingSystem *bs;
    std::string e_name;

public:
    CancelEvent(BookingSystem *b, std::string e)
    {
        bs = b;
        e_name = e;
    }

    void execute()
    {
        Event *ev = bs->get_event(e_name);
        if (ev)
        {
            std::vector<Observer *> observers = ev->getObservers();
            for (Observer *o : observers)
            {
                RefundTicket rt(bs, e_name, static_cast<Customer *>(o));
                rt.execute();
            }
            bs->remove_event(e_name);
            std::cout << "~ Event is successfully canceled, all tickets are refunded ~\n";
        }
        else
        {
            std::cout << "!!! Event is inaccessible or nonexistent !!!\n";
        }
    }
};

class ChangeDate : public Comand
{
private:
    BookingSystem *bs;
    std::string e_name;
    Date new_date;

public:
    ChangeDate(BookingSystem *b, std::string e, Date new_d)
    {
        bs = b;
        e_name = e;
        new_date = new_d;
    }

    void execute()
    {
        Event *ev = bs->get_event(e_name);
        if (ev)
        {
            ev->setDate(new_date);
        }
        else
        {
            std::cout << "!!! Event is inaccessible or nonexistent !!!\n";
        }
    }
};

class ChangeStatus : public Comand
{
private:
    BookingSystem *bs;
    std::string e_name;
    bool new_status;

public:
    ChangeStatus(BookingSystem *b, std::string e, bool new_s)
    {
        bs = b;
        e_name = e;
        new_status = new_s;
    }

    void execute()
    {
        Event *ev = bs->get_event(e_name);
        if (ev)
        {
            ev->setStatus(new_status);
        }
        else
        {
            std::cout << "!!! Event is inaccessible or nonexistent !!!\n";
        }
    }
};

class CreateEvent : public Comand
{
private:
    BookingSystem *bs;
    std::string e_name;
    std::size_t tickets;
    Date date;

public:
    CreateEvent(BookingSystem *b, std::string e, Date d)
    {
        bs = b;
        e_name = e;
        date = d;
    }
    void execute()
    {
        bs->add_event(e_name, date, true);
    }
};

class UI
{
private:
    std::vector<Customer *> customers;
    BookingSystem *system;
    Customer *get_customer(const std::string &name)
    {
        for (auto c : customers)
        {
            if (c->getName() == name)
                return c;
        }
        Customer *new_c = new Customer(name);
        customers.push_back(new_c);
        return new_c;
    }

public:
    UI(BookingSystem *sys) : system(sys) {}
    ~UI()
    {
        for (Customer *c : customers)
        {
            delete c;
        }
    }

    void book()
    {
        std::map<std::string, Event> events = system->get_events();
        std::cout << "Choose event to book: ";
        for (const auto &pair : events)
        {
            std::cout << pair.first << " - " << (pair.second.getStatus() ? "Available" : "Fully Booked") << "\n";
        }
        std::string choice;
        std::cin >> choice;
        std::cout << "Enter your name: ";
        std::string name;
        std::cin >> name;
        Customer *customer = get_customer(name);
        BookTicket bt(system, choice, customer);
        bt.execute();
    }
    void cancel()
    {
        std::map<std::string, Event> events = system->get_events();
        std::cout << "Choose event to cancel: ";
        for (const auto &pair : events)
        {
            std::cout << pair.first << " - " << (pair.second.getStatus() ? "Available" : "Fully Booked") << "\n";
        }
        std::string choice;
        std::cin >> choice;
        CancelEvent ce(system, choice);
        ce.execute();
    }
    void refund()
    {
        std::map<std::string, Event> events = system->get_events();
        std::cout << "Choose event to refund: ";
        for (const auto &pair : events)
        {
            std::cout << pair.first << " - " << (pair.second.getStatus() ? "Available" : "Fully Booked") << "\n";
        }
        std::string choice;
        std::cin >> choice;
        std::cout << "Enter your name: ";
        std::string name;
        std::cin >> name;
        Customer *customer = get_customer(name);
        RefundTicket rt(system, choice, customer);
        rt.execute();
        std::cout << "would you like to unsubscribe from this event? (1 - yes, 0 - no): ";
        bool unsub;
        std::cin >> unsub;
        if (unsub)
        {
            Unsubscribe us(system, choice, customer);
            us.execute();
        }
    }
    void subscribe()
    {
        std::map<std::string, Event> events = system->get_events();
        std::cout << "Choose event to subscribe: ";
        for (const auto &pair : events)
        {
            std::cout << pair.first << " - " << (pair.second.getStatus() ? "Available" : "Fully Booked") << "\n";
        }
        std::string choice;
        std::cin >> choice;
        std::cout << "Enter your name: ";
        std::string name;
        std::cin >> name;
        Customer *customer = get_customer(name);
        Subscribe s(system, choice, customer);
        s.execute();
    }
    void unsubscribe()
    {
        std::map<std::string, Event> events = system->get_events();
        std::cout << "Choose event to unsubscribe: ";
        for (const auto &pair : events)
        {
            std::cout << pair.first << " - " << (pair.second.getStatus() ? "Available" : "Fully Booked") << "\n";
        }
        std::string choice;
        std::cin >> choice;
        std::cout << "Enter your name: ";
        std::string name;
        std::cin >> name;
        Customer *customer = get_customer(name);
        Unsubscribe us(system, choice, customer);
        us.execute();
    }
    void create_event()
    {
        std::cout << "Enter event name: ";
        std::string name;
        std::cin >> name;
        std::cout << "Enter number of tickets: ";
        std::size_t tickets;
        std::cin >> tickets;
        std::cout << "Enter event date (day month year): ";
        Date date;
        std::cin >> date.day >> date.month >> date.year;
        CreateEvent ce(system, name, date);
        ce.execute();
    }
    void change_date()
    {
        std::map<std::string, Event> events = system->get_events();
        std::cout << "Choose event to change date: ";
        for (const auto &pair : events)
        {
            std::cout << pair.first << " - " << (pair.second.getStatus() ? "Available" : "Fully Booked") << "\n";
        }
        std::string choice;
        std::cin >> choice;
        std::cout << "Enter new date (day month year): ";
        Date date;
        std::cin >> date.day >> date.month >> date.year;
        ChangeDate cd(system, choice, date);
        cd.execute();
    }
    void change_status()
    {
        std::map<std::string, Event> events = system->get_events();
        std::cout << "Choose event to change status: ";
        for (const auto &pair : events)
        {
            std::cout << pair.first << " - " << (pair.second.getStatus() ? "Available" : "Fully Booked") << "\n";
        }
        std::string choice;
        std::cin >> choice;
        std::cout << "Enter new status (1 - Available, 0 - Fully Booked): ";
        bool status;
        std::cin >> status;
        ChangeStatus cs(system, choice, status);
        cs.execute();
    }
    void exit()
    {
        std::cout << "Exiting...\n";
    }

    void main_menu()
    {
        while (true)
        {
            std::cout << "1. Book ticket\n2. Cancel event\n3. Refund ticket\n4. Exit\nChoose an option: ";
            int option;
            std::cin >> option;
            switch (option)
            {
            case 1:
                subscribe();
                break;
            case 2:
                unsubscribe();
                break;
            case 3:
                book();
                break;
            case 4:
                refund();
                break;
            case 5:
                change_date();
                break;
            case 6:
                change_status();
                break;
            case 7:
                create_event();
                break;
            case 8:
                cancel();
                break;
            default:
                std::cout << "Invalid option, try again.\n";
            }
        }
    }
};
