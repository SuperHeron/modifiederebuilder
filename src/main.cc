#include "MERCommandLine.hh"

#include <fstream>
#include <iostream>
#include <sstream>

#include "pstream.h"

int main(int argc, char * argv[])
{
    MERCommandLine::get_instance()->run(argc, argv, "mer", "MER_OPTIONS", "MER_CMDLINE");
    std::vector<std::tr1::shared_ptr<const paludis::PackageID> > pkg_to_rebuild;
    std::tr1::shared_ptr<paludis::Environment> env(paludis::EnvironmentFactory::get_instance()->create(MERCommandLine::get_instance()->a_environment.argument()));
    std::tr1::shared_ptr<paludis::PackageIDSequence> ids = (*env)[paludis::selection::AllVersionsSorted(paludis::generator::InRepository(paludis::RepositoryName("installed")))];
    for(paludis::PackageIDSequence::ConstIterator pkgID(ids->begin()), pkgID_end(ids->end()); pkgID != pkgID_end; ++pkgID)
    {
        paludis::FSEntry vdb_dir((*pkgID)->fs_location_key()->value());
        std::ostringstream E_installed_name_ss;
        E_installed_name_ss << (*pkgID)->name().package() << "-" << (*pkgID)->version() << ".*";
        std::ostringstream find_cmd_ss;
        find_cmd_ss << "find " << vdb_dir << " -iname " << E_installed_name_ss.str();
        redi::ipstream find_input(find_cmd_ss.str());
        std::string E_installed_path;
        find_input >> E_installed_path;
        const std::tr1::shared_ptr<const paludis::MetadataCollectionKey<paludis::Set<std::string > > > from_repo_key((*pkgID)->from_repositories_key());
        std::string from_repo_name(paludis::join(from_repo_key->value()->begin(), from_repo_key->value()->end(), " "));
        paludis::VersionRequirement pkg_version_req = { paludis::value_for<paludis::n::version_operator>(paludis::VersionOperator("=")), paludis::value_for<paludis::n::version_spec>((*pkgID)->version()) };
        try
        {
            std::tr1::shared_ptr<paludis::PackageIDSequence> from_ids = (*env)[paludis::selection::RequireExactlyOne(paludis::generator::Matches(paludis::make_package_dep_spec().package((*pkgID)->name()).version_requirement(pkg_version_req).in_repository(paludis::RepositoryName(from_repo_name)), paludis::MatchPackageOptions()))];
            for(paludis::PackageIDSequence::ConstIterator pkgID_from(from_ids->begin()), pkgID_from_end(from_ids->end()); pkgID_from != pkgID_from_end; ++pkgID_from)
            {
                paludis::FSEntry Epath((*pkgID_from)->fs_location_key()->value());
                std::ostringstream diff_cmd_ss;
                diff_cmd_ss << "diff -Nu " << E_installed_path << " " << Epath << " | wc -l";
                redi::ipstream diff_input(diff_cmd_ss.str());
                int diff_lines = 0;
                diff_input >> diff_lines;
                if(diff_lines > 0)
                    pkg_to_rebuild.push_back(*pkgID_from);
            }
        }
        catch(const paludis::DidNotGetExactlyOneError& e)
        {
            std::cerr << e.message() << std::endl;
        }
    }
    for(std::vector<std::tr1::shared_ptr<const paludis::PackageID> >::iterator pkg(pkg_to_rebuild.begin()), pkg_end(pkg_to_rebuild.end()); pkg != pkg_end; ++pkg)
        std::cout << (*pkg)->canonical_form(paludis::idcf_full) << std::endl;
	if(pkg_to_rebuild.size() == 0)
		std::cout << "Nothing to rebuild" << std::endl;
	else
	{
		std::ostringstream paludis_command_ss;
		paludis_command_ss << "paludis -pi1";
		if(MERCommandLine::get_instance()->a_resume_command_template.specified())
			paludis_command_ss << " --" << MERCommandLine::get_instance()->a_resume_command_template.long_name() << " " << MERCommandLine::get_instance()->a_resume_command_template.argument();
		for(std::vector<std::tr1::shared_ptr<const paludis::PackageID> >::iterator pkg(pkg_to_rebuild.begin()), pkg_end(pkg_to_rebuild.end()); pkg != pkg_end; ++pkg)
			paludis_command_ss << " '=" << (*pkg)->canonical_form(paludis::idcf_full) << "'";
		std::cout << paludis_command_ss.str() << std::endl;
		if(paludis::run_command(paludis::Command(paludis_command_ss.str())) == 0)
			std::cout << "Paludis command ran successfully" << std::endl;
		else
			std::cout << "Paludis command didn't run successfully" << std::endl;
	}
	return EXIT_SUCCESS;
}
