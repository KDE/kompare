
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>

#include "kdiffapp.h"
#include "kmaindiffview.h"

static const char *description =
	I18N_NOOP("KDiff");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE
	
	
static KCmdLineOptions options[] =
{
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{

  KAboutData aboutData( "kdiff", I18N_NOOP("KDiff"),
    "2.0", "A program to view the differences between files with.", KAboutData::License_GPL,
    "(c) 2001, Otto Bruggeman", 0, 0, "otto.bruggeman@home.nl");
  aboutData.addAuthor("Otto Bruggeman",0, "otto.bruggeman@home.nl");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KDiffApp* kdiffapp = new KDiffApp();

  return kdiffapp->exec();
}
