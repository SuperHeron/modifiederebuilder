#ifndef __MER_COMMAND_LINE_HH__
#define __MER_COMMAND_LINE_HH__

#include <paludis/args/args.hh>
#include <paludis/args/log_level_arg.hh>
#include <paludis/paludis.hh>

class MERCommandLine :
    public paludis::args::ArgsHandler,
    public paludis::Singleton<MERCommandLine>
{
    friend class paludis::Singleton<MERCommandLine>;

    private:
        MERCommandLine();
        ~MERCommandLine();

    public:
        virtual void run(const int, const char * const * const, const std::string & client, const std::string & env_var, const std::string & env_prefix);

        ///\name Program information
        ///\{

        virtual std::string app_name() const;
        virtual std::string app_synopsis() const;
        virtual std::string app_description() const;

        ///\}

        ///\name Action arguments
        ///\{

        paludis::args::ArgsGroup action_args;
        paludis::args::SwitchArg a_version;
        paludis::args::SwitchArg a_help;

        ///\}

        ///\name General arguments
        ///{

        paludis::args::ArgsGroup general_args;
        paludis::args::LogLevelArg a_log_level;
        paludis::args::StringArg a_environment;
        paludis::args::SwitchArg a_dump_command;
        paludis::args::StringArg a_resume_file;
        paludis::args::IntegerArg a_take_first;

        ///}
};

/**
* Show a '--help' message, and exit.
*/
void show_help_and_exit(const char * const argv[]) PALUDIS_ATTRIBUTE((noreturn));

/**
* Show a '--version' message, and exit.
*/
void show_version_and_exit() PALUDIS_ATTRIBUTE((noreturn));

#endif // __MER_COMMAND_LINE_HH__
