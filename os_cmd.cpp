#include "mine_vector.hpp"
#include <iostream>
#include <sstream>
#include <map>
#include <ctime>
#include <vector>

class Folder;
class FileSystem;

class FSItem
{
public:
    size_t size = 0; // bytes
    std::string name;
    time_t created_at;
    time_t modified_at;

    FSItem()
    {
        created_at = time(nullptr);
        modified_at = time(nullptr);
    }
    virtual ~FSItem() = default;

    virtual std::string get_type() const { return "Item"; }
    virtual std::string print_tree(int indent = 0) const { return name; }

    bool operator==(const FSItem &o) const { return name == o.name && get_type() == o.get_type(); }
    bool operator!=(const FSItem &o) const { return !(*this == o); }
};

class File : public FSItem
{
private:
    std::string content;

public:
    File() {}
    File(std::string n, std::string c = "")
    {
        name = std::move(n);
        content = std::move(c);
        size = content.size();
    }

    std::string get_type() const override { return "File"; }
    std::string get_content() const { return content; }
    void set_content(const std::string &c)
    {
        content = c;
        size = c.size();
        modified_at = time(nullptr);
    }

    std::string print_tree(int indent = 0) const override
    {
        std::string branch;
        if (indent > 0)
        {
            for (int i = 0; i < indent - 1; ++i)
                branch += "-   ";
            branch += "|-- ";
        }
        return branch + name;
    }
};

class Folder : public FSItem
{
private:
    Folder *parent = nullptr;
    Vector<FSItem *> items;

    std::string resolve_name(const std::string &base, const std::string &type) const
    {
        std::string candidate = base;
        int cnt = 1;
        while (find_item(candidate, type) != nullptr)
            candidate = base + " (" + std::to_string(cnt++) + ")";
        return candidate;
    }

public:
    Folder() {}
    Folder(std::string n) { name = std::move(n); }

    ~Folder()
    {
        for (size_t i = 0; i < items.size(); ++i)
            delete items[i];
    }

    std::string get_type() const override { return "Folder"; }

    void set_parent(Folder *p) { parent = p; }
    Folder *get_parent() const { return parent; }

    FSItem *find_item(const std::string &n, const std::string &type = "") const
    {
        for (size_t i = 0; i < items.size(); ++i)
        {
            if (items[i]->name == n && (type.empty() || items[i]->get_type() == type))
                return items[i];
        }
        return nullptr;
    }

    void add_file(const std::string &n, size_t s = 0, const std::string &content = "")
    {
        std::string uname = resolve_name(n, "File");
        File *f = new File(uname, content);

        items.push_back(f);
    }

    void add_folder(const std::string &n)
    {
        std::string uname = resolve_name(n, "Folder");
        Folder *d = new Folder(uname);
        d->set_parent(this);

        items.push_back(d);
    }

    // вытащить указатель из вектора и отдать новому владельцу
    FSItem *detach_item(const std::string &n)
    {
        for (size_t i = 0; i < items.size(); ++i)
        {
            if (items[i]->name == n)
            {
                FSItem *ptr = items[i]; // вытащили указатель на нужный элемент

                for (size_t j = i; j + 1 < items.size(); ++j) // пересыпали все последующие
                    items[j] = items[j + 1];

                items.pop_back(); // уменьшили размер
                return ptr;
            }
        }
        return nullptr;
    }

    bool remove_item(const std::string &n)
    {
        FSItem *p = detach_item(n);
        if (!p)
            return false;
        delete p;
        return true;
    }

    bool remove_folder_if_empty(const std::string &n)
    {
        FSItem *p = find_item(n, "Folder");
        if (!p)
            return false;
        Folder *d = static_cast<Folder *>(p);
        if (d->items.size() != 0)
            return false;
        return remove_item(n);
    }

    size_t child_count() const { return items.size(); }
    FSItem *child_at(size_t i) const { return items.at(i); }

    std::string print_path() const
    {
        if (!parent || parent == this)
            return "/" + name;
        std::string p = parent->print_path();
        if (p == "/")
            return "/" + name;
        return p + "/" + name;
    }

    std::string print_tree(int indent = 0) const override
    {
        std::string out;
        if (indent == 0)
        {
            out = name + "\n";
        }
        else
        {
            std::string branch;
            for (int i = 0; i < indent - 1; ++i)
                branch += "|   ";
            out = branch + "|-- " + name + "\n";
        }
        for (size_t i = 0; i < items.size(); ++i)
        {
            std::string branch;
            bool is_last = (i == items.size() - 1);
            if (items[i]->get_type() == "Folder")
            {
                out += items[i]->print_tree(indent + 1);
            }
            else
            {
                std::string branch;
                for (int k = 0; k < indent; ++k)
                    branch += "|   ";
                out += branch + (is_last ? "L-- " : "|-- ") + items[i]->name + "\n";
            }
        }
        return out;
    }

    // for ls
    std::string list() const
    {
        if (items.size() == 0)
        {
            return "(directory seems to be empty)\n";
        }
        std::string out;
        for (size_t i = 0; i < items.size(); ++i)
        {
            out += (items[i]->get_type() == "Folder" ? "[DIR] " : "[FILE]");
            out += " " + items[i]->name + "\n";
        }
        return out;
    }
};

//  TemplateStore
class TemplateStore
{
private:
    std::map<std::string, std::string> templates;

public:
    bool create(const std::string &name, const std::string &content)
    {
        if (templates.count(name))
            return false;
        templates[name] = content;
        return true;
    }

    bool exists(const std::string &name) const { return templates.count(name) != 0; }

    std::string get(const std::string &name) const
    {
        auto it = templates.find(name);
        return (it != templates.end()) ? it->second : "";
    }

    bool remove(const std::string &name) { return templates.erase(name) != 0; }

    std::string list() const
    {
        if (templates.empty())
            return "(no templates)\n";
        std::string out;
        for (auto &kv : templates)
            out += "  " + kv.first + "\n";
        return out;
    }
};

class FileSystem
{
private:
    Folder root;
    Folder *cwd = &root; // Current Working Directory
    TemplateStore tmpl_store;

    FileSystem()
    {
        root = Folder("root");
        root.set_parent(&root);
    }

    FileSystem(const FileSystem &) = delete;
    FileSystem &operator=(const FileSystem &) = delete;

    Folder *navigate(const std::string &path)
    {
        if (path.empty() || path == ".")
            return cwd;
        if (path == "..")
        {
            Folder *p = cwd->get_parent();
            return (p && p != cwd) ? p : cwd;
        }

        //  пикаем начальную папку для рекурсивного спуска по пути
        Folder *cur = nullptr;
        if (path[0] == '/')
            cur = &root;
        else
            cur = cwd;

        std::istringstream ss(path);
        std::string token;

        while (std::getline(ss, token, '/'))
        {
            if (token.empty() || token == ".")
                continue;
            if (token == "..")
            {
                Folder *p = cur->get_parent();
                cur = (p && p != cur) ? p : cur;
                continue;
            }
            FSItem *item = cur->find_item(token, "Folder");
            if (!item)
                return nullptr;
            cur = static_cast<Folder *>(item);
        }
        return cur;
    }

public:
    static FileSystem &instance()
    {
        static FileSystem fs;
        return fs;
    }

    std::string pwd() const { return cwd->print_path(); }

    std::string cd(const std::string &path)
    {
        Folder *dest = navigate(path);
        if (!dest)
            return "cd: " + path + ": No such directory\n";
        cwd = dest;
        return "";
    }

    std::string ls(const std::string path = ".")
    {
        Folder *target = navigate(path);
        if (target)
            return target->list();
        return "ls: " + path + " : No such directory";
    }

    std::string tree(const std::string path = ".")
    {
        Folder *target = navigate(path);
        if (target)
            return target->print_tree();
        return "tree: " + path + " : No such directory";
    }

    std::string mkdir(const std::string &name)
    {
        if (cwd->find_item(name))
            return "mkdir: " + name + ": already exists\n";
        cwd->add_folder(name);
        return "";
    }

    std::string rmdir(const std::string &name)
    {
        if (!cwd->remove_folder_if_empty(name))
            return "rmdir: " + name + ": not found or not empty\n";
        return "";
    }

    std::string touch(const std::string &name)
    {
        FSItem *target = cwd->find_item(name, "File");
        if (target)
        {
            target->modified_at = time(nullptr);
            return "";
        }
        cwd->add_file(name);
        return "";
    }

    std::string cp(const std::string &src, const std::string &dst_name)
    {
        FSItem *s = cwd->find_item(src, "File");
        if (!s)
            return "cp: " + src + ": not found\n";
        File *f = static_cast<File *>(s);
        cwd->add_file(dst_name, f->size, f->get_content());
        return "";
    }

    std::string mv(const std::string &src, const std::string &dst)
    {
        FSItem *s = cwd->find_item(src);
        if (!s)
            return "mv: " + src + ": not found\n";

        Folder *dest_dir = navigate(dst);
        if (dest_dir)
        {
            FSItem *item = cwd->detach_item(src);
            if (!item)
                return "mv: error\n";
            if (item->get_type() == "Folder")
            {
                dest_dir->add_folder(item->name);
                delete item;
            }
            else
            {
                File *f = static_cast<File *>(item);
                dest_dir->add_file(f->name, f->size, f->get_content());
                delete f;
            }
        }
        else
        {
            s->name = dst;
        }
        return "";
    }

    std::string rm(const std::string &name, bool recursive = false)
    {
        FSItem *item = cwd->find_item(name);
        if (!item)
            return "rm: " + name + ": not found\n";
        if (item->get_type() == "Folder" && !recursive)
            return "rm: " + name + ": is a directory (use rm -rf)\n";
        cwd->remove_item(name);
        return "";
    }

    std::string cat(const std::string &name)
    {
        FSItem *item = cwd->find_item(name, "File");
        if (!item)
            return "cat: " + name + ": not found\n";
        std::string c = static_cast<File *>(item)->get_content();
        return c.empty() ? "(empty file)\n" : c + "\n";
    }

    std::string find_(const Folder *dir, const std::string &pattern, const std::string &prefix) const
    {
        std::string out;
        for (size_t i = 0; i < dir->child_count(); ++i)
        {
            FSItem *item = dir->child_at(i);
            std::string full = prefix + "/" + item->name;
            if (item->name.find(pattern) != std::string::npos)
                out += full + "\n";
            if (item->get_type() == "Folder")
                out += find_(static_cast<Folder *>(item), pattern, full);
        }
        return out;
    }

    std::string find(const std::string &pattern) const
    {
        std::string out = find_(cwd, pattern, cwd->print_path());
        return out.empty() ? "(not found)\n" : out;
    }

    // template create <name> <content>
    std::string template_create(const std::string &name, const std::string &content)
    {
        if (!tmpl_store.create(name, content))
            return "template: '" + name + "' already exists\n";
        return "Template '" + name + "' created.\n";
    }

    // template list
    std::string template_list() const { return tmpl_store.list(); }

    // template apply <name> <file>
    std::string template_apply(const std::string &tmpl_name, const std::string &file_name)
    {
        if (!tmpl_store.exists(tmpl_name))
            return "template: '" + tmpl_name + "' not found\n";
        std::string content = tmpl_store.get(tmpl_name);
        touch(file_name);
        FSItem *item = cwd->find_item(file_name, "File");
        if (!item)
            return "template: failed to create file\n";
        static_cast<File *>(item)->set_content(content);
        return "File '" + file_name + "' created from template '" + tmpl_name + "'.\n";
    }

    // rename <pattern> <replacement>
    std::string rename_pattern(const std::string &from, const std::string &to)
    {
        int count = 0;
        for (size_t i = 0; i < cwd->child_count(); ++i)
        {
            FSItem *item = cwd->child_at(i);
            std::string &n = item->name;
            size_t pos = n.find(from);
            if (pos != std::string::npos)
            {
                n.replace(pos, from.size(), to);
                ++count;
            }
        }
        return "Renamed " + std::to_string(count) + " item(s).\n";
    }

    // batch <commands_text>
    std::string batch_exec(const std::string &commands); //      Console
};

class Console
{
private:
    FileSystem &fs;
    bool running = true;

    static std::vector<std::string> tokenize(const std::string &line)
    {
        std::vector<std::string> tokens;
        std::istringstream ss(line);
        std::string tok;
        while (ss >> tok)
            tokens.push_back(tok);
        return tokens;
    }


    static std::string join_from(const std::vector<std::string> &t, size_t start)
    {
        std::string r;
        for (size_t i = start; i < t.size(); ++i)
        {
            if (i > start)
                r += " ";
            r += t[i];
        }
        return r;
    }

public:
    explicit Console(FileSystem &f) : fs(f) {}

    std::string execute(const std::string &line)
    {
        if (line.empty() || line[0] == '#')
            return "";
        auto t = tokenize(line);
        if (t.empty())
            return "";

        const std::string &cmd = t[0];


        if (cmd == "pwd")
            return fs.pwd() + "\n";
        if (cmd == "ls")
            return fs.ls();
        if (cmd == "tree")
            return fs.tree();

        if (cmd == "cd")
        {
            if (t.size() < 2)
                return "cd: missing argument\n";
            return fs.cd(t[1]);
        }


        if (cmd == "mkdir")
        {
            if (t.size() < 2)
                return "mkdir: missing name\n";
            return fs.mkdir(t[1]);
        }
        if (cmd == "rmdir")
        {
            if (t.size() < 2)
                return "rmdir: missing name\n";
            return fs.rmdir(t[1]);
        }


        if (cmd == "touch")
        {
            if (t.size() < 2)
                return "touch: missing name\n";
            return fs.touch(t[1]);
        }
        if (cmd == "cp")
        {
            if (t.size() < 3)
                return "cp: usage: cp <src> <dst>\n";
            return fs.cp(t[1], t[2]);
        }
        if (cmd == "mv")
        {
            if (t.size() < 3)
                return "mv: usage: mv <src> <dst>\n";
            return fs.mv(t[1], t[2]);
        }
        if (cmd == "rm")
        {
            if (t.size() < 2)
                return "rm: missing argument\n";
            bool rf = (t[1] == "-rf");
            std::string name = rf ? (t.size() > 2 ? t[2] : "") : t[1];
            if (name.empty())
                return "rm: missing name\n";
            return fs.rm(name, rf);
        }


        if (cmd == "cat")
        {
            if (t.size() < 2)
                return "cat: missing name\n";
            return fs.cat(t[1]);
        }
        if (cmd == "find")
        {
            if (t.size() < 2)
                return "find: missing pattern\n";
            return fs.find(t[1]);
        }


        if (cmd == "template")
        {
            if (t.size() < 2)
                return "template: usage: template <create|apply|list> ...\n";
            if (t[1] == "create")
            {
                if (t.size() < 4)
                    return "template create: usage: template create <name> <content...>\n";
                return fs.template_create(t[2], join_from(t, 3));
            }
            if (t[1] == "apply")
            {
                if (t.size() < 4)
                    return "template apply: usage: template apply <name> <file>\n";
                return fs.template_apply(t[2], t[3]);
            }
            if (t[1] == "list")
                return fs.template_list();
            return "template: unknown subcommand '" + t[1] + "'\n";
        }

        if (cmd == "rename")
        {
            if (t.size() < 3)
                return "rename: usage: rename <from> <to>\n";
            return fs.rename_pattern(t[1], t[2]);
        }

        if (cmd == "batch")
        {
            if (t.size() < 2)
                return "batch: usage: batch <commands_file>\n";

            std::string content = fs.cat(t[1]);
            if (content.rfind("cat:", 0) == 0)
                return content; 
            if (!content.empty() && content.back() == '\n')
                content.pop_back();
            return batch_run(content);
        }


        if (cmd == "help")
            return help_text();
        if (cmd == "exit")
        {
            running = false;
            return "Bye!\n";
        }

        return cmd + ": command not found\n";
    }

    std::string batch_run(const std::string &script)
    {
        std::string out;
        std::istringstream ss(script);
        std::string line;
        while (std::getline(ss, line))
        {
            std::string res = execute(line);
            if (!res.empty())
                out += ">> " + line + "\n" + res;
        }
        return out.empty() ? "(batch: no output)\n" : out;
    }


    bool is_running() const { return running; }

    static std::string help_text()
    {
        return "Navigation:  pwd  cd <path>  ls  tree\n"
               "Directories: mkdir <name>  rmdir <name>\n"
               "Files:       touch <name>  cp <src> <dst>  mv <src> <dst>\n"
               "             rm [-rf] <name>  cat <name>  find <pattern>\n"
               "Templates:   template create <name> <content>\n"
               "             template apply  <name> <file>\n"
               "             template list\n"
               "Batch:       rename <from> <to>  batch <file>\n"
               "Other:       help  exit\n";
    }
};


int main()
{
    FileSystem &fs = FileSystem::instance();
    Console console(fs);

    std::cout << "FileSystem shell. Type 'help' for commands, 'exit' to quit.\n";

    while (console.is_running())
    {
        std::cout << fs.pwd() << " $ ";
        std::string line;
        if (!std::getline(std::cin, line))
            break;
        std::string result = console.execute(line);
        if (!result.empty())
            std::cout << result;
    }

    return 0;
}