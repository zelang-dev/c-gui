#ifndef MW_MOWITZ_H
#define MW_MOWITZ_H

#ifdef USE_DEBUG
#	undef MOWITZ_DATA
#	undef DEFAULT_PIXPATH
#	undef DEFAULT_DATAPATH
#	define MOWITZ_DATA "./share"
#	define DEFAULT_PIXPATH "./share/pixmaps"
#	define DEFAULT_DATAPATH MOWITZ_DATA
#else
#	ifndef MOWITZ_DATA
#		define MOWITZ_DATA "/usr/local/share"
#	endif
#	ifndef DEFAULT_PIXPATH
#		define DEFAULT_PIXPATH "/usr/local/share/pixmaps"
#	endif
#	ifndef DEFAULT_DATAPATH
#		define DEFAULT_DATAPATH MOWITZ_DATA
#	endif
#endif

#include <Linux/Mowitz/MwUtils.h>
#include <Linux/Mowitz/MwFormat.h>
#include <Linux/Mowitz/MwXutils.h>
#include <Linux/Mowitz/MwXFormat.h>
#include <Linux/Mowitz/MwAnimator.h>
#include <Linux/Mowitz/MwApplicationShell.h>
#include <Linux/Mowitz/MwBase.h>
#include <Linux/Mowitz/MwBaseComp.h>
#include <Linux/Mowitz/MwBaseConst.h>
#include <Linux/Mowitz/MwBaseME.h>
#include <Linux/Mowitz/MwButton.h>
#include <Linux/Mowitz/MwCanvas.h>
#include <Linux/Mowitz/MwCheck.h>
#include <Linux/Mowitz/MwCheckME.h>
#include <Linux/Mowitz/MwCombo.h>
#include <Linux/Mowitz/MwDialog.h>
#include <Linux/Mowitz/MwDND.h>
#include <Linux/Mowitz/MwFilesel.h>
#include <Linux/Mowitz/MwFrame.h>
#include <Linux/Mowitz/MwHandle.h>
#include <Linux/Mowitz/MwImage.h>
#include <Linux/Mowitz/MwLabelME.h>
#include <Linux/Mowitz/MwLineME.h>
#include <Linux/Mowitz/MwListTree.h>
#include <Linux/Mowitz/MwMBButton.h>
#include <Linux/Mowitz/MwMenu.h>
#include <Linux/Mowitz/MwMenuBar.h>
#include <Linux/Mowitz/MwMenuButton.h>
#include <Linux/Mowitz/MwNotebook.h>
#include <Linux/Mowitz/MwPopText.h>
#include <Linux/Mowitz/MwRichtext.h>
#include <Linux/Mowitz/MwRow.h>
#include <Linux/Mowitz/MwRudegrid.h>
#include <Linux/Mowitz/MwRuler.h>
#include <Linux/Mowitz/MwSButton.h>
#include <Linux/Mowitz/MwSlider.h>
#include <Linux/Mowitz/MwSpinner.h>
#include <Linux/Mowitz/MwSubME.h>
#include <Linux/Mowitz/MwTabbing.h>
#include <Linux/Mowitz/MwTable.h>
#include <Linux/Mowitz/MwTabs.h>
#include <Linux/Mowitz/MwTabstop.h>
#include <Linux/Mowitz/MwTextField.h>
#include <Linux/Mowitz/MwTooltip.h>
#include <Linux/Mowitz/MwTraverse.h>
#include <Linux/Mowitz/MwVSlider.h>
#include <Linux/Mowitz/MwXCC.h>
#include <Linux/Mowitz/MwHtml.h>
#endif	/* MW_MOWITZ_H */
