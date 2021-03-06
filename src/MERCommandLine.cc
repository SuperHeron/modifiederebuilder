#include "MERCommandLine.hh"

#include <iostream>

template class paludis::Singleton<MERCommandLine>;

MERCommandLine::MERCommandLine() :
    ArgsHandler(),

    action_args(this->main_options_section(), "Actions", "Selects which basic action to perform. At most one action should be specified."),
    a_version(&action_args, "version", 'V',  "Display program version", false),
    a_help(&action_args, "help", 'h',  "Display program help", false),

    general_args(this->main_options_section(), "General options", "Options which are relevant for most or all actions."),
    a_log_level(&general_args, "log-level", '\0'),
    a_environment(&general_args, "environment", 'E', "Environment specification (class:suffix, both parts optional)"),
	a_dump_command(&general_args, "dump-command", '\0', "Dump build command at end of MER process (if successful)", false),
    a_resume_file(&general_args, "resume-file", '\0', "Save the resume command to a file to use with 'cave resume'."),
    a_take_first(&general_args, "take-first", '\0', "Take the n first paquets")
{}

std::string MERCommandLine::app_name() const
{
    return "ModifiedERebuilder";
}

std::string MERCommandLine::app_synopsis() const
{
    return "Application to rebuild modified E's";
}

std::string MERCommandLine::app_description() const
{
    return "This program will rebuild all the modified E's.";
}

MERCommandLine::~MERCommandLine()
{
}

void show_help_and_exit(const char * const argv[])
{
    std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
    std::cout << std::endl;
    std::cout << *MERCommandLine::get_instance();

    std::exit(EXIT_SUCCESS);
}

void show_version_and_exit()
{
    std::cout << "ModifiedERebuilder 1.0" << std::endl;

    std::exit(EXIT_SUCCESS);
}

void MERCommandLine::run(const int argc, const char * const * const argv, const std::string & client, const std::string & env_var, const std::string & env_prefix)
{
    paludis::args::ArgsHandler::run(argc, argv, client, env_var, env_prefix);

    if (MERCommandLine::get_instance()->a_log_level.specified())
        paludis::Log::get_instance()->set_log_level(MERCommandLine::get_instance()->a_log_level.option());

    if (MERCommandLine::get_instance()->a_help.specified())
        show_help_and_exit(argv);

    if (MERCommandLine::get_instance()->a_version.specified())
        show_version_and_exit();

}
