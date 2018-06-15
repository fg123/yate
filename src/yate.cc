#include "yate.h"
#include "tab-set.h"

Yate::Yate() : Yate((Config){})
{

}

Yate::Yate(Config config) : config(config)
{
	root = new PaneSet();
	root->addPane(new TabSet());
}

Yate::~Yate()
{
	delete root;
}
