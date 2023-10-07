#include <Windows.h>
#include <libds/heap_monitor.h>
#include "DataIO.h"
#include "IS.h"
#include "HierarchySVK.h"
#include "Tables.h"
#include "Sort.h"

int main()
{
	initHeapMonitor();
	SetConsoleOutputCP(1250);
	SetConsoleCP(1250);

	ImplicitSequences IS = ImplicitSequences();

	ds::amt::ImplicitSequence<Unit*> regions{ IS.getRegions() };
	ds::amt::ImplicitSequence<Unit*> districts{ IS.getDistricts() };
	ds::amt::ImplicitSequence<Unit*> municipalities{ IS.getMunicipalities() };

	HierarchySVK hierarchySVK = HierarchySVK<ds::amt::ImplicitSequence<Unit*>>(regions, districts, municipalities);

	Tables tables = Tables<Unit, ds::amt::ImplicitSequence<Unit*>>(regions, districts, municipalities);

	size_t cont{ 1 };
	while (cont)
	{
		size_t level;
		InputCheck().checkInput(level, "Vyberte úroveò: sekvencie [1] | hierarchia + triedenia [2/4] | tabu¾ky [3]: ", "Nevhodný vstup. Zadajte znova: ",
			[&level]() -> bool { return level != 1 && level != 2 && level != 3 && level != 4; });

		switch (level)
		{
		case 1:
			IS.findAndProcessUnit();					// 1. uroven
			break;

		case 2:
		case 4:
			hierarchySVK.navigateHierarchy();			// 2. + 4. uroven
			break;

		case 3:
			tables.displayUnitInfo();					// 3. uroven
			break;
		}
		InputCheck().checkInput(cont, "Chcete pokraèova ïalšou úrovòou? [0/1]: ", "Nevhodný vstup. Zadajte znova: ",
			[&cont]() -> bool { return cont != 0 && cont != 1; });
	}

	return 0;
}