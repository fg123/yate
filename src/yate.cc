#include "yate.h"
#include "tab-set.h"
#include "logging.h"

Yate::Yate() : Yate((Config){})
{

}

Yate::Yate(Config config) : config(config)
{
	Logging::info("=== Starting Yate ===");
	root = new PaneSet();
	root->addPane(new TabSet());
	root->draw();
	getch();
}

Yate::~Yate()
{
	delete root;
}
