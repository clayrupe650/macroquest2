#include "../MQ2Plugin.h"
#include "MQ2Map3.h"

// ***************************************************************************
// Function:    MapFilters
// Description: Our '/mapfilter' command
//              Sets map filters
// Usage:       /mapfilter [options|help]
// ***************************************************************************
VOID MapFilterSetting(PSPAWNINFO pChar, DWORD nMapFilter, PCHAR szValue)
{
    CHAR szBuffer[MAX_STRING] = {0};
    CHAR Buff[MAX_STRING]={0};
    DWORD dwValue = 0;
    PCHAR szFilterMap[] = {
        "hide",
        "show",
        NULL
    };
    if (!pChar) return;
	PMAPFILTER pMapFilter=&MapFilterOptions[nMapFilter];
	if (!RequirementsMet(nMapFilter))
	{
		sprintf(szBuffer,"'%s' requires '%s' option.  Please enable this option first.",pMapFilter->szName,MapFilterOptions[pMapFilter->RequiresOption].szName);
		WriteChatColor(szBuffer,USERCOLOR_DEFAULT);
		return;
	}
    if (!szValue) {
        if (pMapFilter->bIsToggle) {
            sprintf(szBuffer,"%s: %s",pMapFilter->szName,szFilterMap[pMapFilter->Enabled]);
        } else if (nMapFilter == MAPFILTER_Custom) {
            if (IsOptionEnabled(nMapFilter)==0) {
                sprintf(szBuffer,"%s: Off",pMapFilter->szName);
            } else {
                sprintf(szBuffer,"%s: %s",pMapFilter->szName,FormatSearchSpawn(Buff,&MapFilterCustom));
            }
        } else {
            sprintf(szBuffer,"%s: %d",pMapFilter->szName,pMapFilter->Enabled);
        }
        if (pMapFilter->DefaultColor != -1) {
            CHAR szBuffer2[MAX_STRING] = {0};
            DWORD R,G,B;
            R= (pMapFilter->Color&0xFF0000)/0x10000;
            G= (pMapFilter->Color&0xFF00)/0x100;
            B= pMapFilter->Color&0xFF;
            sprintf(szBuffer2,"%s (Color: %d %d %d)",szBuffer,R,G,B);
            strcpy(szBuffer,szBuffer2);
        }
    } else {
        if (pMapFilter->bIsToggle) {
                        if (!stricmp(szFilterMap[0],szValue)) {
                            pMapFilter->Enabled = 0;
                        } else if (!stricmp(szFilterMap[1],szValue)) {
                            pMapFilter->Enabled = 1;
                        } else {
                pMapFilter->Enabled = 1 - pMapFilter->Enabled;
                        }
            sprintf(szBuffer,"%s is now set to: %s",pMapFilter->szName,szFilterMap[IsOptionEnabled(nMapFilter)]);
        } else if (nMapFilter == MAPFILTER_Custom) {
            ClearSearchSpawn(&MapFilterCustom);
            if (szValue[0]==0) {
                pMapFilter->Enabled = 0;
                sprintf(szBuffer,"%s is now set to: Off",pMapFilter->szName);
            } else {
                pMapFilter->Enabled = 1;
                ParseSearchSpawn(szValue,&MapFilterCustom);
                sprintf(szBuffer,"%s is now set to: %s",pMapFilter->szName,FormatSearchSpawn(Buff,&MapFilterCustom));
            }
        } else {
            pMapFilter->Enabled = atoi(szValue);
            sprintf(szBuffer,"%s is now set to: %d",pMapFilter->szName,pMapFilter->Enabled);
        }
    }
    WriteChatColor(szBuffer,USERCOLOR_DEFAULT);
    if (szValue) {
        itoa(pMapFilter->Enabled,szBuffer,10);
        WritePrivateProfileString("Map Filters",pMapFilter->szName,szBuffer,INIFileName);
    }
}

VOID MapFilters(PSPAWNINFO pChar, PCHAR szLine)
{
    bRunNextCommand = TRUE;
    CHAR szArg[MAX_STRING] = {0};
    GetArg(szArg,szLine,1);
    PCHAR szRest = GetNextArg(szLine);
    CHAR szBuffer[MAX_STRING] = {0};

    // Display settings
    if (szArg[0]==0) {
        WriteChatColor("Map filtering settings:",USERCOLOR_DEFAULT);
		WriteChatColor("-----------------------",USERCOLOR_DEFAULT);
        for (DWORD i=0;MapFilterOptions[i].szName!=NULL;i++) 
			if (RequirementsMet(i))
				MapFilterSetting(pChar,i);

    // Display Help
    } else if (!strnicmp(szArg,"help",6)) {
        WriteChatColor("Map filtering options:",USERCOLOR_DEFAULT);
        for (DWORD i=0;MapFilterOptions[i].szName!=NULL;i++) {
            sprintf(szBuffer,"%s%s: %s",MapFilterOptions[i].szName,(MapFilterOptions[i].bIsToggle)?"":" #",MapFilterOptions[i].szHelpString);
            WriteChatColor(szBuffer,USERCOLOR_DEFAULT);
        }
        WriteChatColor("'option' color [r g b]: Set display color for 'option' (Omit to reset to default)",USERCOLOR_DEFAULT);

    // Set option
    } else {
        PMAPFILTER Found = 0;
        for (DWORD i=0;MapFilterOptions[i].szName!=NULL;i++) {
            if (!stricmp(szArg,MapFilterOptions[i].szName)) {
                if (!strnicmp(szRest,"color",5)) {
                    if (MapFilterOptions[i].DefaultColor == -1) {
                        sprintf(szBuffer,"Option '%s' does not have a color.",MapFilterOptions[i].szName);
                        WriteChatColor(szBuffer,USERCOLOR_DEFAULT);
                    } else {
                        DWORD R,G,B;
                        CHAR szBuffer2[MAX_STRING] = {0};
                        GetArg(szArg,szRest,2);
                        if (szArg[0]==0) {
                            MapFilterOptions[i].Color = MapFilterOptions[i].DefaultColor;
                        } else {
                            R=atoi(szArg);
                            G=atoi(GetArg(szArg,szRest,3));
                            B=atoi(GetArg(szArg,szRest,4));
                            if (R>255) R=255;
                            if (G>255) G=255;
                            if (B>255) B=255;
                           MapFilterOptions[i].Color = R*0x10000 + G*0x100 + B;
                        }
                        sprintf(szBuffer,"Option '%s' color set to: %d %d %d",MapFilterOptions[i].szName,R,G,B);
                        WriteChatColor(szBuffer,USERCOLOR_DEFAULT);
                        itoa(MapFilterOptions[i].Color & 0xFFFFFF,szBuffer,10);
                        sprintf(szBuffer2,"%s-Color",MapFilterOptions[i].szName);
                        WritePrivateProfileString("Map Filters",szBuffer2,szBuffer,INIFileName);
                        MapFilterOptions[i].Color |= 0xFF000000;
                    }
                } else {
                    MapFilterSetting(pChar,i,szRest);
                }
                Found=&MapFilterOptions[i];
            }
        }
        if (!Found) 
			WriteChatColor("Usage: /mapfilter [option|help]",USERCOLOR_DEFAULT);
		else if (Found->RegenerateOnChange)
		{
			MapClear();
			MapGenerate();
		}
    }
}


VOID MapHighlightCmd(PSPAWNINFO pChar, PCHAR szLine)
{
    CHAR szArg[MAX_STRING] = {0};
    CHAR szBuffer[MAX_STRING] = {0};
    bRunNextCommand = TRUE;
	if (szLine==0 || szLine[0]==0)
	{
		WriteChatColor("Usage: /highlight [spawnfilter|[color # # #]]",USERCOLOR_DEFAULT);
		return;
	};

    GetArg(szArg,szLine,1);
	if (!stricmp(szArg,"color"))
	{
		GetArg(szArg,szLine,2);
		if (szLine[0]==0)
		{
			sprintf(szBuffer,"Highlight color: %d %d %d",(HighlightColor&0x00FF0000)>>16,(HighlightColor&0x0000FF00)>>8,(HighlightColor&0x000000FF));
			WriteChatColor(szBuffer);
			return;
		}
        unsigned char R=atoi(szArg);
        unsigned char G=atoi(GetArg(szArg,szLine,3));
        unsigned char B=atoi(GetArg(szArg,szLine,4));
		HighlightColor=0xFF000000 | (R << 16) | (G << 8) | (B);
		sprintf(szBuffer,"Highlight color: %d %d %d",R,G,B);
		WriteChatColor(szBuffer);
		return;
	}
	else if (!stricmp(szArg,"reset"))
	{
		/*
		PMAPSPAWN pMapSpawn=pActiveSpawns;
		while(pMapSpawn)
		{
			pMapSpawn->Highlight=false;
			pMapSpawn=pMapSpawn->pNext;
		}
		/**/
		MapHighlight(0);
		WriteChatColor("Highlighting reset",USERCOLOR_DEFAULT);
		return;
	}

	if (PCHARINFO pCharInfo=GetCharInfo())
	{
		SEARCHSPAWN ssHighlight;
		ClearSearchSpawn(&ssHighlight);
		ParseSearchSpawn(szLine,&ssHighlight);
		/*
		PMAPSPAWN pMapSpawn=pActiveSpawns;
		unsigned long Count=0;
		while(pMapSpawn)
		{
			// update!
			if (SpawnMatchesSearch(&ssHighlight,pCharInfo->pSpawn,pMapSpawn->pSpawn))
			{
				//pMapSpawn->pMapLabel->Color.ARGB=HighlightColor;
				pMapSpawn->Highlight=true;
				Count++;
			}

			pMapSpawn=pMapSpawn->pNext;
		}
		/**/
		sprintf(szBuffer,"%d mapped spawns highlighted",MapHighlight(&ssHighlight));
		WriteChatColor(szBuffer,USERCOLOR_DEFAULT);
	}
}


VOID MapNames(PSPAWNINFO pChar, PCHAR szLine)
{
	CHAR szOut[MAX_STRING]={0};
	if (!szLine[0])
	{
		sprintf(szOut,"Normal naming string: %s",MapNameString);
		WriteChatColor(szOut,USERCOLOR_DEFAULT);
		sprintf(szOut,"Target naming string: %s",MapTargetNameString);
		WriteChatColor(szOut,USERCOLOR_DEFAULT);
		return;
	}
    CHAR szArg[MAX_STRING] = {0};
    GetArg(szArg,szLine,1);
    PCHAR szRest = GetNextArg(szLine);
	if (!stricmp(szArg,"target"))
	{
		if (!stricmp(szRest,"reset"))
			strcpy(MapTargetNameString,"%N");
		else
			strcpy(MapTargetNameString,szRest);
		sprintf(szOut,"Target naming string: %s",MapTargetNameString);
		WriteChatColor(szOut,USERCOLOR_DEFAULT);
		WritePrivateProfileString("Naming Schemes","Target",MapNameString,INIFileName);
		MapClear();
		MapGenerate();
	}
	else if (!stricmp(szArg,"normal"))
	{
		if (!stricmp(szRest,"reset"))
			strcpy(MapNameString,"%N");
		else
			strcpy(MapNameString,szRest);
		sprintf(szOut,"Normal naming string: %s",MapNameString);
		WriteChatColor(szOut,USERCOLOR_DEFAULT);
		WritePrivateProfileString("Naming Schemes","Normal",MapNameString,INIFileName);
		MapClear();
		MapGenerate();
	}
	else
	{
		WriteChatColor("Usage: /mapnames <target|normal> [value|reset]");
	}
}