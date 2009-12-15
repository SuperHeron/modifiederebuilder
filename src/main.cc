#include "MERCommandLine.hh"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#include "pstream.h"

std::map<std::string, std::string> loadEEnvironment(paludis::FSEntry& vdb_dir)
{
	std::map<std::string, std::string> envVars;
	std::ostringstream bunzip2_cmd_ss;
	std::string input_line;
	bunzip2_cmd_ss << "bzip2 -dc " << vdb_dir << "/environment.bz2";
//	std::cout << bunzip2_cmd_ss.str() << std::endl;
	redi::ipstream bunzip2_input(bunzip2_cmd_ss.str());
	while(getline(bunzip2_input, input_line))
	{
		if(input_line.length() > 0)
		{
//			std::cout << input_line << std::endl;
			unsigned int equalPos = input_line.find("=");
			if(equalPos != std::string::npos)
			{
//				std::cout << input_line.substr(0, equalPos) << " = " << input_line.substr(equalPos + 1) << std::endl;
				envVars[input_line.substr(0, equalPos)] = input_line.substr(equalPos + 1);
			}
		}
	}
	return envVars;
}

int main(int argc, char * argv[])
{
    MERCommandLine::get_instance()->run(argc, argv, "mer", "MER_OPTIONS", "MER_CMDLINE");
    std::vector<std::tr1::shared_ptr<const paludis::PackageID> > pkg_to_rebuild;
    std::tr1::shared_ptr<paludis::Environment> env(paludis::EnvironmentFactory::get_instance()->create(MERCommandLine::get_instance()->a_environment.argument()));
    std::tr1::shared_ptr<paludis::PackageIDSequence> ids = (*env)[paludis::selection::AllVersionsSorted(paludis::generator::InRepository(paludis::RepositoryName("installed")))];
//    std::cout << env->distribution() << std::endl;
    for(paludis::PackageIDSequence::ConstIterator pkgID(ids->begin()), pkgID_end(ids->end()); pkgID != pkgID_end; ++pkgID)
    {
//    	std::cout << (*pkgID)->canonical_form(paludis::idcf_full) << std::endl;
        paludis::FSEntry vdb_dir((*pkgID)->fs_location_key()->value());
		std::map<std::string, std::string> environment = loadEEnvironment(vdb_dir);
		paludis::FSEntry E_installed_path(vdb_dir);
		std::ostringstream Ename;
		if(env->distribution() == "gentoo")
			Ename << (*pkgID)->name().package() << "-" << (*pkgID)->version() << ".ebuild";
		else
			Ename << (*pkgID)->name().package() << "-" << (*pkgID)->version() << "." << environment["EAPI"];
		E_installed_path /= Ename.str();
//		std::cout << E_installed_path << std::endl;
		paludis::FSEntry E_from_repo_path(environment["EBUILD"]);
//		std::cout << E_from_repo_path << std::endl;
		std::ostringstream diff_cmd_ss;
		redi::ipstream diff_input;
		if(E_from_repo_path.exists())
		{
			diff_cmd_ss << "diff -Nu " << E_installed_path << " " << E_from_repo_path << " | wc -l";
			diff_input.open(diff_cmd_ss.str());
		}
		int diff_lines_Esource = 0;
		diff_input >> diff_lines_Esource;
//		std::cout << "diff_lines_Esource = " << diff_lines_Esource << std::endl;
		int diff_Elibs = 0;
		std::istringstream Elibs_dirs;
		std::istringstream Elibs_names;
		if(env->distribution() == "gentoo")
			Elibs_dirs.str(environment["ECLASSDIRS"].substr(environment["ECLASSDIRS"].find_first_not_of(" '$"), environment["ECLASSDIRS"].find_last_not_of(" '$") - environment["ECLASSDIRS"].find_first_not_of(" '$") + 1));
		else
			Elibs_dirs.str(environment["EXLIBSDIRS"].substr(environment["EXLIBSDIRS"].find_first_not_of(" '$"), environment["EXLIBSDIRS"].find_last_not_of(" '$") - environment["EXLIBSDIRS"].find_first_not_of(" '$") + 1));
//		std::cout << Elibs_dirs.str() << std::endl;
		if(environment.find("INHERITED") != environment.end())
		{
			Elibs_names.str(environment["INHERITED"].substr(environment["INHERITED"].find_first_not_of(" '$"), environment["INHERITED"].find_last_not_of(" '$") - environment["INHERITED"].find_first_not_of(" '$") + 1));
			std::string dir;
			while(Elibs_dirs >> dir)
			{
				std::string name;
				while(Elibs_names >> name)
				{
					paludis::FSEntry Elib_entry(dir);
					if(env->distribution() == "gentoo")
						Elib_entry /= name + ".eclass";
					else
						Elib_entry /= name + ".exlib";
//					std::cout << Elib_entry << "(" << std::boolalpha << Elib_entry.exists() << std::noboolalpha << ")" << std::endl;
					if(Elib_entry.exists())
					{
//						std::cout << Elib_entry.mtim() << "|" << E_installed_path.mtim() << std::endl;
						if(E_installed_path.mtim() < Elib_entry.mtim())
							diff_Elibs++;
					}
				}
			}
		}
//		std::cout << "diff_Elibs = " << diff_Elibs << std::endl;
		std::ostringstream E_from_repo_patches_str;
		if(env->distribution() == "gentoo")
			E_from_repo_patches_str << environment["FILESDIR"];
		else
			E_from_repo_patches_str << environment["FILES"];
		paludis::FSEntry E_from_repo_patches(E_from_repo_patches_str.str());
		int diff_patches = 0;
		if(E_from_repo_patches.exists())
		{
			for(paludis::DirIterator di(E_from_repo_patches), di_end; di != di_end; di++)
			{
				if(E_installed_path.mtim() < di->mtim())
				{
//					std::cout << *di << " mtime : " << di->mtim() << std::endl;
//					std::cout << E_installed_path << " mtime : " << E_installed_path.mtim() << std::endl;
					diff_patches++;
				}
			}
		}
		if(E_from_repo_path.exists() && (diff_lines_Esource > 0 || diff_Elibs > 0 || diff_patches > 0))
		{
			std::tr1::shared_ptr<paludis::PackageIDSequence> pkgIDFromSequence((*env)[paludis::selection::AllVersionsSorted(paludis::generator::Intersection(
																						  paludis::generator::Package((*pkgID)->name()),
																						  paludis::generator::InRepository(paludis::RepositoryName(environment["REPOSITORY"]))) |
																					  paludis::filter::SameSlot((*pkgID))
																					  )]);
			std::tr1::shared_ptr<const paludis::PackageID> pkgIDFrom;
			for(paludis::PackageIDSequence::ConstIterator fromPkgID(pkgIDFromSequence->begin()), fromPkgID_end(pkgIDFromSequence->end()); fromPkgID != fromPkgID_end; ++fromPkgID)
			{
				if((*fromPkgID)->version() == (*pkgID)->version())
					pkgIDFrom = *fromPkgID;
			}
			pkg_to_rebuild.push_back(pkgIDFrom);
		}
    }
    for(std::vector<std::tr1::shared_ptr<const paludis::PackageID> >::iterator pkg(pkg_to_rebuild.begin()), pkg_end(pkg_to_rebuild.end()); pkg != pkg_end; ++pkg)
		std::cout << (*pkg)->canonical_form(paludis::idcf_full) << std::endl;
	if(pkg_to_rebuild.size() == 0)
		std::cout << "Nothing to rebuild" << std::endl;
	else
	{
		std::cout << pkg_to_rebuild.size() << " packages to rebuild" << std::endl;
		unsigned int arg_first = MERCommandLine::get_instance()->a_take_first.specified() ? MERCommandLine::get_instance()->a_take_first.argument() : 0;
		if(arg_first > 0 && pkg_to_rebuild.size() > arg_first)
			std::cout << "Rebuilding the " << arg_first << " firsts..." << std::endl;
		std::ostringstream paludis_command_ss;
		paludis_command_ss << "paludis --pretend --install --preserve-world";
		if(MERCommandLine::get_instance()->a_log_level.specified())
			paludis_command_ss << " --" << MERCommandLine::get_instance()->a_log_level.long_name() << " " << MERCommandLine::get_instance()->a_log_level.argument();
		if(MERCommandLine::get_instance()->a_resume_command_template.specified())
			paludis_command_ss << " --" << MERCommandLine::get_instance()->a_resume_command_template.long_name() << " " << MERCommandLine::get_instance()->a_resume_command_template.argument();
		unsigned int count = 0;
		for(std::vector<std::tr1::shared_ptr<const paludis::PackageID> >::iterator pkg(pkg_to_rebuild.begin()), pkg_end(pkg_to_rebuild.end()); pkg != pkg_end; ++pkg)
		{
			paludis_command_ss << " '=" << (*pkg)->canonical_form(paludis::idcf_full) << "'";
			count++;
			if(arg_first > 0 && count == arg_first)
				break;
		}
		std::cout << paludis_command_ss.str() << std::endl;
		if(paludis::run_command(paludis::Command(paludis_command_ss.str())) == 0)
			std::cout << "Paludis command ran successfully" << std::endl;
		else
			std::cout << "Paludis command didn't run successfully" << std::endl;
	}
	return EXIT_SUCCESS;
}
