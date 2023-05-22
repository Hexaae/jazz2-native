﻿#if defined(WITH_ANGELSCRIPT)

#include "LevelScriptLoader.h"
#include "RegisterRef.h"
#include "RegisterString.h"
#include "RegisterArray.h"
#include "RegisterDictionary.h"
#include "ScriptActorWrapper.h"
#include "ScriptPlayerWrapper.h"

#include "../LevelHandler.h"
#include "../PreferencesCache.h"
#include "../Actors/ActorBase.h"
#include "../Actors/Player.h"
#include "../Compatibility/JJ2Strings.h"

#include "../../nCine/Base/Random.h"

namespace Jazz2::Scripting
{
	static void Unimplemented(const char* sourceName) {
		auto ctx = asGetActiveContext();
		if (ctx != nullptr) {
			const char* sectionName;
			int lineNumber = ctx->GetLineNumber(0, nullptr, &sectionName);
			LOGW("%s (called from \"%s:%i\")", sourceName, sectionName, lineNumber);
		} else {
			LOGW("%s", sourceName);
		}
	}

#if defined(DEATH_TARGET_GCC)
#	define noop() Unimplemented(__PRETTY_FUNCTION__)
#elif defined(DEATH_TARGET_MSVC)
#	define noop() Unimplemented(__FUNCTION__)
#else
#	define noop() Unimplemented(__func__)
#endif

	enum airjump {
		airjumpNONE,
		airjumpHELICOPTER,
		airjumpSPAZ
	};

	enum ambientLighting {
		ambientLighting_OPTIONAL,
		ambientLighting_BASIC,
		ambientLighting_COMPLETE
	};

	enum anim {
		mAMMO,
		mBAT,
		mBEEBOY,
		mBEES,
		mBIGBOX,
		mBIGROCK,
		mBIGTREE,
		mBILSBOSS,
		mBIRD,
		mBIRD3D,
		mBOLLPLAT,
		mBONUS,
		mBOSS,
		mBRIDGE,
		mBUBBA,
		mBUMBEE,
		mBUTTERFLY,
		mCARROTPOLE,
		mCAT,
		mCAT2,
		mCATERPIL,
		mCHUCK,
		mCOMMON,
		mCONTINUE,
		mDEMON,
		mDESTSCEN,
		mDEVAN,
		mDEVILDEVAN,
		mDIAMPOLE,
		mDOG,
		mDOOR,
		mDRAGFLY,
		mDRAGON,

		mEVA,
		mFACES,

		mFATCHK,
		mFENCER,
		mFISH,
		mFLAG,
		mFLARE,
		mFONT,
		mFROG,
		mFRUITPLAT,
		mGEMRING,
		mGLOVE,
		mGRASSPLAT,
		mHATTER,
		mHELMUT,

		mJAZZ,
		mJAZZ3D,

		mJUNGLEPOLE,
		mLABRAT,
		mLIZARD,
		mLORI,
		mLORI2,

		mMENU,
		mMENUFONT,

		mMONKEY,
		mMOTH,

		mPICKUPS,
		mPINBALL,
		mPINKPLAT,
		mPSYCHPOLE,
		mQUEEN,
		mRAPIER,
		mRAVEN,
		mROBOT,
		mROCK,
		mROCKTURT,

		mSKELETON,
		mSMALTREE,
		mSNOW,
		mSONCSHIP,
		mSONICPLAT,
		mSPARK,
		mSPAZ,
		mSPAZ2,
		mSPAZ3D,
		mSPIKEBOLL,
		mSPIKEBOLL3D,
		mSPIKEPLAT,
		mSPRING,
		mSTEAM,

		mSUCKER,
		mTUBETURT,
		mTUFBOSS,
		mTUFTURT,
		mTURTLE,
		mTWEEDLE,
		mUTERUS,
		mVINE,
		mWARP10,
		mWARP100,
		mWARP20,
		mWARP50,

		mWITCH,
		mXBILSY,
		mXLIZARD,
		mXTURTLE,
		mZDOG,
		mZSPARK,
		mZZAMMO,
		mZZBETA,
		mZZCOMMON,
		mZZCONTINUE,

		mZZFONT,
		mZZMENUFONT,
		mZZREPLACEMENTS,
		mZZRETICLES,
		mZZSCENERY,
		mZZWARP,

		mCOUNT
	};

	enum dir {
		dirRIGHT,
		dirLEFT,
		dirUP,
		dirCURRENT
	};

	enum gameState {
		gameSTOPPED,
		gameSTARTED,
		gamePAUSED,
		gamePREGAME,
		gameOVERTIME
	};

	enum gameConnection {
		gameLOCAL,
		gameINTERNET,
		gameLAN_TCP
	};

	enum GM_ {
		GM_SP,
		GM_COOP,
		GM_BATTLE,
		GM_CTF,
		GM_TREASURE,
		GM_RACE
	};

	enum groundjump {
		groundjumpNONE,
		groundjumpREGULARJUMP,
		groundjumpJAZZ,
		groundjumpSPAZ,
		groundjumpLORI
	};

	enum object {
		aUNKNOWN,

		aPLAYERBULLET1,
		aPLAYERBULLET2,
		aPLAYERBULLET3,
		aPLAYERBULLET4,
		aPLAYERBULLET5,
		aPLAYERBULLET6,
		aPLAYERBULLET8,
		aPLAYERBULLET9,
		aPLAYERBULLETP1,
		aPLAYERBULLETP2,
		aPLAYERBULLETP3,
		aPLAYERBULLETP4,
		aPLAYERBULLETP5,
		aPLAYERBULLETP6,
		aPLAYERBULLETP8,
		aPLAYERBULLETP9,
		aPLAYERBULLETC1,
		aPLAYERBULLETC2,
		aPLAYERBULLETC3,
		aBULLET,
		aCATSMOKE,
		aSHARD,
		aEXPLOSION,
		aBOUNCEONCE,
		aREDGEMTEMP,
		aPLAYERLASER,
		aUTERUSEL,
		aBIRD,
		aBUBBLE,
		aGUN3AMMO3,
		aGUN2AMMO3,
		aGUN4AMMO3,
		aGUN5AMMO3,
		aGUN6AMMO3,
		aGUN7AMMO3,
		aGUN8AMMO3,
		aGUN9AMMO3,
		aTURTLESHELL,
		aSWINGVINE,
		aBOMB,
		aSILVERCOIN,
		aGOLDCOIN,
		aGUNCRATE,
		aCARROTCRATE,
		a1UPCRATE,
		aGEMBARREL,
		aCARROTBARREL,
		a1UPBARREL,
		aBOMBCRATE,
		aGUN3AMMO15,
		aGUN2AMMO15,
		aGUN4AMMO15,
		aGUN5AMMO15,
		aGUN6AMMO15,
		aTNT,
		aAIRBOARDGENERATOR,
		aFROZENGREENSPRING,
		aGUNFASTFIRE,
		aSPRINGCRATE,
		aREDGEM,
		aGREENGEM,
		aBLUEGEM,
		aPURPLEGEM,
		aSUPERREDGEM,
		aBIRDCAGE,
		aGUNBARREL,
		aGEMCRATE,
		aMORPHMONITOR,
		aENERGYUP,
		aFULLENERGY,
		aFIRESHIELD,
		aWATERSHIELD,
		aLIGHTSHIELD,
		aFASTFEET,
		aEXTRALIFE,
		aENDOFLEVELPOST,
		aSAVEPOST,
		aBONUSLEVELPOST,
		aREDSPRING,
		aGREENSPRING,
		aBLUESPRING,
		aINVINCIBILITY,
		aEXTRATIME,
		aFREEZER,
		aHREDSPRING,
		aHGREENSPRING,
		aHBLUESPRING,
		aBIRDMORPHMONITOR,
		aTRIGGERCRATE,
		aFLYCARROT,
		aRECTREDGEM,
		aRECTGREENGEM,
		aRECTBLUEGEM,
		aTUFTURT,
		aTUFBOSS,
		aLABRAT,
		aDRAGON,
		aLIZARD,
		aBUMBEE,
		aRAPIER,
		aSPARK,
		aBAT,
		aSUCKER,
		aCATERPILLAR,
		aCHESHIRE1,
		aCHESHIRE2,
		aHATTER,
		aBILSYBOSS,
		aSKELETON,
		aDOGGYDOGG,
		aNORMTURTLE,
		aHELMUT,
		aDEMON,
		aDRAGONFLY,
		aMONKEY,
		aFATCHK,
		aFENCER,
		aFISH,
		aMOTH,
		aSTEAM,
		aROCK,
		aGUN1POWER,
		aGUN2POWER,
		aGUN3POWER,
		aGUN4POWER,
		aGUN5POWER,
		aGUN6POWER,
		aPINLEFTPADDLE,
		aPINRIGHTPADDLE,
		aPIN500BUMP,
		aPINCARROTBUMP,
		aAPPLE,
		aBANANA,
		aCHERRY,
		aORANGE,
		aPEAR,
		aPRETZEL,
		aSTRAWBERRY,
		aSTEADYLIGHT,
		aPULZELIGHT,
		aFLICKERLIGHT,
		aQUEENBOSS,
		aFLOATSUCKER,
		aBRIDGE,
		aLEMON,
		aLIME,
		aTHING,
		aWMELON,
		aPEACH,
		aGRAPES,
		aLETTUCE,
		aEGGPLANT,
		aCUCUMB,
		aCOKE,
		aPEPSI,
		aMILK,
		aPIE,
		aCAKE,
		aDONUT,
		aCUPCAKE,
		aCHIPS,
		aCANDY1,
		aCHOCBAR,
		aICECREAM,
		aBURGER,
		aPIZZA,
		aFRIES,
		aCHICKLEG,
		aSANDWICH,
		aTACOBELL,
		aWEENIE,
		aHAM,
		aCHEESE,
		aFLOATLIZARD,
		aSTANDMONKEY,
		aDESTRUCTSCENERY,
		aDESTRUCTSCENERYBOMB,
		aCOLLAPSESCENERY,
		aSTOMPSCENERY,
		aGEMSTOMP,
		aRAVEN,
		aTUBETURTLE,
		aGEMRING,
		aROTSMALLTREE,
		aAMBIENTSOUND,
		aUTERUS,
		aCRAB,
		aWITCH,
		aROCKTURT,
		aBUBBA,
		aDEVILDEVAN,
		aDEVANROBOT,
		aROBOT,
		aCARROTUSPOLE,
		aPSYCHPOLE,
		aDIAMONDUSPOLE,
		aFRUITPLATFORM,
		aBOLLPLATFORM,
		aGRASSPLATFORM,
		aPINKPLATFORM,
		aSONICPLATFORM,
		aSPIKEPLATFORM,
		aSPIKEBOLL,
		aGENERATOR,
		aEVA,
		aBUBBLER,
		aTNTPOWER,
		aGUN8POWER,
		aGUN9POWER,
		aSPIKEBOLL3D,
		aSPRINGCORD,
		aBEES,
		aCOPTER,
		aLASERSHIELD,
		aSTOPWATCH,
		aJUNGLEPOLE,
		aBIGROCK,
		aBIGBOX,
		aTRIGGERSCENERY,
		aSONICBOSS,
		aBUTTERFLY,
		aBEEBOY,
		aSNOW,
		aTWEEDLEBOSS,
		aAIRBOARD,
		aFLAG,
		aXNORMTURTLE,
		aXLIZARD,
		aXFLOATLIZARD,
		aXBILSYBOSS,
		aZCAT,
		aZGHOST,

		areaONEWAY,
		areaHURT,
		areaVINE,
		areaHOOK,
		areaSLIDE,
		areaHPOLE,
		areaVPOLE,
		areaFLYOFF,
		areaRICOCHET,
		areaBELTRIGHT,
		areaBELTLEFT,
		areaBELTACCRIGHT,
		areaBELTACCLEFT,
		areaSTOPENEMY,
		areaWINDLEFT,
		areaWINDRIGHT,
		areaEOL,
		areaWARPEOL,
		areaENDMORPH,
		areaFLOATUP,
		areaROCKTRIGGER,
		areaDIMLIGHT,
		areaSETLIGHT,
		areaLIMITXSCROLL,
		areaRESETLIGHT,
		areaWARPSECRET,
		areaECHO,
		areaBOSSTRIGGER,
		areaJAZZLEVELSTART,
		areaSPAZLEVELSTART,
		areaMPLEVELSTART,
		areaLORILEVELSTART,
		areaWARP,
		areaWARPTARGET,
		areaAREAID,
		areaNOFIREZONE,
		areaTRIGGERZONE,

		aSUCKERTUBE,
		aTEXT,
		aWATERLEVEL,
		aMORPHFROG,
		aWATERBLOCK,

		aCOUNT
	};

	enum particle {
		particleNONE,
		particlePIXEL,
		particleFIRE,
		particleSMOKE,
		particleICETRAIL,
		particleSPARK,
		particleSCORE,
		particleSNOW,
		particleRAIN,
		particleFLOWER,
		particleLEAF,
		particleSTAR,
		particleTILE
	};

	enum playerAnim {
		mJAZZ_AIRBOARD,
		mJAZZ_AIRBOARDTURN,
		mJAZZ_BUTTSTOMPLAND,
		mJAZZ_CORPSE,
		mJAZZ_DIE,
		mJAZZ_DIVE,
		mJAZZ_DIVEFIREQUIT,
		mJAZZ_DIVEFIRERIGHT,
		mJAZZ_DIVEUP,
		mJAZZ_EARBRACHIATE,
		mJAZZ_ENDOFLEVEL,
		mJAZZ_FALL,
		mJAZZ_FALLBUTTSTOMP,
		mJAZZ_FALLLAND,
		mJAZZ_FIRE,
		mJAZZ_FIREUP,
		mJAZZ_FIREUPQUIT,
		mJAZZ_FROG,
		mJAZZ_HANGFIREQUIT,
		mJAZZ_HANGFIREREST,
		mJAZZ_HANGFIREUP,
		mJAZZ_HANGIDLE1,
		mJAZZ_HANGIDLE2,
		mJAZZ_HANGINGFIREQUIT,
		mJAZZ_HANGINGFIRERIGHT,
		mJAZZ_HELICOPTER,
		mJAZZ_HELICOPTERFIREQUIT,
		mJAZZ_HELICOPTERFIRERIGHT,
		mJAZZ_HPOLE,
		mJAZZ_HURT,
		mJAZZ_IDLE1,
		mJAZZ_IDLE2,
		mJAZZ_IDLE3,
		mJAZZ_IDLE4,
		mJAZZ_IDLE5,
		mJAZZ_JUMPFIREQUIT,
		mJAZZ_JUMPFIRERIGHT,
		mJAZZ_JUMPING1,
		mJAZZ_JUMPING2,
		mJAZZ_JUMPING3,
		mJAZZ_LEDGEWIGGLE,
		mJAZZ_LIFT,
		mJAZZ_LIFTJUMP,
		mJAZZ_LIFTLAND,
		mJAZZ_LOOKUP,
		mJAZZ_LOOPY,
		mJAZZ_PUSH,
		mJAZZ_QUIT,
		mJAZZ_REV1,
		mJAZZ_REV2,
		mJAZZ_REV3,
		mJAZZ_RIGHTFALL,
		mJAZZ_RIGHTJUMP,
		mJAZZ_ROLLING,
		mJAZZ_RUN1,
		mJAZZ_RUN2,
		mJAZZ_RUN3,
		mJAZZ_SKID1,
		mJAZZ_SKID2,
		mJAZZ_SKID3,
		mJAZZ_SPRING,
		mJAZZ_STAND,
		mJAZZ_STATIONARYJUMP,
		mJAZZ_STATIONARYJUMPEND,
		mJAZZ_STATIONARYJUMPSTART,
		mJAZZ_STONED,
		mJAZZ_SWIMDOWN,
		mJAZZ_SWIMRIGHT,
		mJAZZ_SWIMTURN1,
		mJAZZ_SWIMTURN2,
		mJAZZ_SWIMUP,
		mJAZZ_SWINGINGVINE,
		mJAZZ_TELEPORT,
		mJAZZ_TELEPORTFALL,
		mJAZZ_TELEPORTFALLING,
		mJAZZ_TELEPORTFALLTELEPORT,
		mJAZZ_TELEPORTSTAND,
		mJAZZ_VPOLE
	};

	enum spriteType {
		spriteType_NORMAL,
		spriteType_TRANSLUCENT,
		spriteType_TINTED,
		spriteType_GEM,
		spriteType_INVISIBLE,
		spriteType_SINGLECOLOR,
		spriteType_RESIZED,
		spriteType_NEONGLOW,
		spriteType_FROZEN,
		spriteType_PLAYER,
		spriteType_PALSHIFT,
		spriteType_SHADOW,
		spriteType_SINGLEHUE,
		spriteType_BRIGHTNESS,
		spriteType_TRANSLUCENTCOLOR,
		spriteType_TRANSLUCENTPLAYER,
		spriteType_TRANSLUCENTPALSHIFT,
		spriteType_TRANSLUCENTSINGLEHUE,
		spriteType_ALPHAMAP,
		spriteType_MENUPLAYER,
		spriteType_BLENDNORMAL,
		spriteType_BLENDDARKEN,
		spriteType_BLENDLIGHTEN,
		spriteType_BLENDHUE,
		spriteType_BLENDSATURATION,
		spriteType_BLENDCOLOR,
		spriteType_BLENDLUMINANCE,
		spriteType_BLENDMULTIPLY,
		spriteType_BLENDSCREEN,
		spriteType_BLENDDISSOLVE,
		spriteType_BLENDOVERLAY,
		spriteType_BLENDHARDLIGHT,
		spriteType_BLENDSOFTLIGHT,
		spriteType_BLENDDIFFERENCE,
		spriteType_BLENDDODGE,
		spriteType_BLENDBURN,
		spriteType_BLENDEXCLUSION,
		spriteType_TRANSLUCENTTILE,
		spriteType_CHROMAKEY,
		spriteType_MAPPING,
		spriteType_TRANSLUCENTMAPPING
	};

	enum sound {
		sAMMO_BLUB1,
		sAMMO_BLUB2,
		sAMMO_BMP1,
		sAMMO_BMP2,
		sAMMO_BMP3,
		sAMMO_BMP4,
		sAMMO_BMP5,
		sAMMO_BMP6,
		sAMMO_BOEM1,
		sAMMO_BUL1,
		sAMMO_BULFL1,
		sAMMO_BULFL2,
		sAMMO_BULFL3,
		sAMMO_FIREGUN1A,
		sAMMO_FIREGUN2A,
		sAMMO_FUMP,
		sAMMO_GUN1,
		sAMMO_GUN2,
		sAMMO_GUN3PLOP,
		sAMMO_GUNFLP,
		sAMMO_GUNFLP1,
		sAMMO_GUNFLP2,
		sAMMO_GUNFLP3,
		sAMMO_GUNFLP4,
		sAMMO_GUNFLPL,
		sAMMO_GUNJAZZ,
		sAMMO_GUNVELOCITY,
		sAMMO_ICEGUN,
		sAMMO_ICEGUN2,
		sAMMO_ICEGUNPU,
		sAMMO_ICEPU1,
		sAMMO_ICEPU2,
		sAMMO_ICEPU3,
		sAMMO_ICEPU4,
		sAMMO_LASER,
		sAMMO_LASER2,
		sAMMO_LASER3,
		sAMMO_LAZRAYS,
		sAMMO_MISSILE,
		sAMMO_SPZBL1,
		sAMMO_SPZBL2,
		sAMMO_SPZBL3,
		sBAT_BATFLY1,
		sBILSBOSS_BILLAPPEAR,
		sBILSBOSS_FINGERSNAP,
		sBILSBOSS_FIRE,
		sBILSBOSS_FIRESTART,
		sBILSBOSS_SCARY3,
		sBILSBOSS_THUNDER,
		sBILSBOSS_ZIP,
		sBONUS_BONUS1,
		sBONUS_BONUSBLUB,
		sBUBBA_BUBBABOUNCE1,
		sBUBBA_BUBBABOUNCE2,
		sBUBBA_BUBBAEXPLO,
		sBUBBA_FROG2,
		sBUBBA_FROG3,
		sBUBBA_FROG4,
		sBUBBA_FROG5,
		sBUBBA_SNEEZE2,
		sBUBBA_TORNADOATTACK2,
		sBUMBEE_BEELOOP,
		sCATERPIL_RIDOE,
		sCOMMON_AIRBOARD,
		sCOMMON_AIRBTURN,
		sCOMMON_AIRBTURN2,
		sCOMMON_BASE1,
		sCOMMON_BELL_FIRE,
		sCOMMON_BELL_FIRE2,
		sCOMMON_BENZIN1,
		sCOMMON_BIRDFLY,
		sCOMMON_BIRDFLY2,
		sCOMMON_BLOKPLOP,
		sCOMMON_BLUB1,
		sCOMMON_BUBBLGN1,
		sCOMMON_BURN,
		sCOMMON_BURNIN,
		sCOMMON_CANSPS,
		sCOMMON_CLOCK,
		sCOMMON_COIN,
		sCOMMON_COLLAPS,
		sCOMMON_CUP,
		sCOMMON_DAMPED1,
		sCOMMON_DOWN,
		sCOMMON_DOWNFL2,
		sCOMMON_DRINKSPAZZ1,
		sCOMMON_DRINKSPAZZ2,
		sCOMMON_DRINKSPAZZ3,
		sCOMMON_DRINKSPAZZ4,
		sCOMMON_EAT1,
		sCOMMON_EAT2,
		sCOMMON_EAT3,
		sCOMMON_EAT4,
		sCOMMON_ELECTRIC1,
		sCOMMON_ELECTRIC2,
		sCOMMON_ELECTRICHIT,
		sCOMMON_EXPL_TNT,
		sCOMMON_EXPSM1,
		sCOMMON_FLAMER,
		sCOMMON_FLAP,
		sCOMMON_FOEW1,
		sCOMMON_FOEW2,
		sCOMMON_FOEW3,
		sCOMMON_FOEW4,
		sCOMMON_FOEW5,
		sCOMMON_GEMSMSH1,
		sCOMMON_GLASS2,
		sCOMMON_GUNSM1,
		sCOMMON_HARP1,
		sCOMMON_HEAD,
		sCOMMON_HELI1,
		sCOMMON_HIBELL,
		sCOMMON_HOLYFLUT,
		sCOMMON_HORN1,
		sCOMMON_ICECRUSH,
		sCOMMON_IMPACT1,
		sCOMMON_IMPACT2,
		sCOMMON_IMPACT3,
		sCOMMON_IMPACT4,
		sCOMMON_IMPACT5,
		sCOMMON_IMPACT6,
		sCOMMON_IMPACT7,
		sCOMMON_IMPACT8,
		sCOMMON_IMPACT9,
		sCOMMON_ITEMTRE,
		sCOMMON_JUMP,
		sCOMMON_JUMP2,
		sCOMMON_LAND,
		sCOMMON_LAND1,
		sCOMMON_LAND2,
		sCOMMON_LANDCAN1,
		sCOMMON_LANDCAN2,
		sCOMMON_LANDPOP,
		sCOMMON_LOADJAZZ,
		sCOMMON_LOADSPAZ,
		sCOMMON_METALHIT,
		sCOMMON_MONITOR,
		sCOMMON_NOCOIN,
		sCOMMON_PICKUP1,
		sCOMMON_PICKUPW1,
		sCOMMON_PISTOL1,
		sCOMMON_PLOOP1,
		sCOMMON_PLOP1,
		sCOMMON_PLOP2,
		sCOMMON_PLOP3,
		sCOMMON_PLOP4,
		sCOMMON_PLOPKORK,
		sCOMMON_PREEXPL1,
		sCOMMON_PREHELI,
		sCOMMON_REVUP,
		sCOMMON_RINGGUN,
		sCOMMON_RINGGUN2,
		sCOMMON_SHIELD1,
		sCOMMON_SHIELD4,
		sCOMMON_SHIELD_ELEC,
		sCOMMON_SHLDOF3,
		sCOMMON_SLIP,
		sCOMMON_SMASH,
		sCOMMON_SPLAT1,
		sCOMMON_SPLAT2,
		sCOMMON_SPLAT3,
		sCOMMON_SPLAT4,
		sCOMMON_SPLUT,
		sCOMMON_SPRING1,
		sCOMMON_STEAM,
		sCOMMON_STEP,
		sCOMMON_STRETCH,
		sCOMMON_SWISH1,
		sCOMMON_SWISH2,
		sCOMMON_SWISH3,
		sCOMMON_SWISH4,
		sCOMMON_SWISH5,
		sCOMMON_SWISH6,
		sCOMMON_SWISH7,
		sCOMMON_SWISH8,
		sCOMMON_TELPORT1,
		sCOMMON_TELPORT2,
		sCOMMON_UP,
		sCOMMON_WATER,
		sCOMMON_WOOD1,
		sDEMON_RUN,
		sDEVILDEVAN_DRAGONFIRE,
		sDEVILDEVAN_FLAP,
		sDEVILDEVAN_FROG4,
		sDEVILDEVAN_JUMPUP,
		sDEVILDEVAN_LAUGH,
		sDEVILDEVAN_PHASER2,
		sDEVILDEVAN_STRECH2,
		sDEVILDEVAN_STRECHTAIL,
		sDEVILDEVAN_STRETCH1,
		sDEVILDEVAN_STRETCH3,
		sDEVILDEVAN_VANISH1,
		sDEVILDEVAN_WHISTLEDESCENDING2,
		sDEVILDEVAN_WINGSOUT,
		sDOG_AGRESSIV,
		sDOG_SNIF1,
		sDOG_WAF1,
		sDOG_WAF2,
		sDOG_WAF3,
		sDRAGFLY_BEELOOP,
		sENDING_OHTHANK,
		sENDTUNEJAZZ_TUNE,
		sENDTUNELORI_CAKE,
		sENDTUNESPAZ_TUNE,
		sEPICLOGO_EPIC1,
		sEPICLOGO_EPIC2,
		sEVA_KISS1,
		sEVA_KISS2,
		sEVA_KISS3,
		sEVA_KISS4,
		sFAN_FAN,
		sFATCHK_HIT1,
		sFATCHK_HIT2,
		sFATCHK_HIT3,
		sFENCER_FENCE1,
		sFROG_FROG,
		sFROG_FROG1,
		sFROG_FROG2,
		sFROG_FROG3,
		sFROG_FROG4,
		sFROG_FROG5,
		sFROG_JAZZ2FROG,
		sFROG_TONG,
		sGLOVE_HIT,
		sHATTER_CUP,
		sHATTER_HAT,
		sHATTER_PTOEI,
		sHATTER_SPLIN,
		sHATTER_SPLOUT,
		sINTRO_BLOW,
		sINTRO_BOEM1,
		sINTRO_BOEM2,
		sINTRO_BRAKE,
		sINTRO_END,
		sINTRO_GRAB,
		sINTRO_GREN1,
		sINTRO_GREN2,
		sINTRO_GREN3,
		sINTRO_GUNM0,
		sINTRO_GUNM1,
		sINTRO_GUNM2,
		sINTRO_HELI,
		sINTRO_HITSPAZ,
		sINTRO_HITTURT,
		sINTRO_IFEEL,
		sINTRO_INHALE,
		sINTRO_INSECT,
		sINTRO_KATROL,
		sINTRO_LAND,
		sINTRO_MONSTER,
		sINTRO_MONSTER2,
		sINTRO_ROCK,
		sINTRO_ROPE1,
		sINTRO_ROPE2,
		sINTRO_RUN,
		sINTRO_SHOT1,
		sINTRO_SHOTGRN,
		sINTRO_SKI,
		sINTRO_STRING,
		sINTRO_SWISH1,
		sINTRO_SWISH2,
		sINTRO_SWISH3,
		sINTRO_SWISH4,
		sINTRO_UHTURT,
		sINTRO_UP1,
		sINTRO_UP2,
		sINTRO_WIND_01,
		sJAZZSOUNDS_BALANCE,
		sJAZZSOUNDS_HEY1,
		sJAZZSOUNDS_HEY2,
		sJAZZSOUNDS_HEY3,
		sJAZZSOUNDS_HEY4,
		sJAZZSOUNDS_IDLE,
		sJAZZSOUNDS_JAZZV1,
		sJAZZSOUNDS_JAZZV2,
		sJAZZSOUNDS_JAZZV3,
		sJAZZSOUNDS_JAZZV4,
		sJAZZSOUNDS_JUMMY,
		sJAZZSOUNDS_PFOE,
		sLABRAT_BITE,
		sLABRAT_EYE2,
		sLABRAT_EYE3,
		sLABRAT_MOUSE1,
		sLABRAT_MOUSE2,
		sLABRAT_MOUSE3,
		sLIZARD_LIZ1,
		sLIZARD_LIZ2,
		sLIZARD_LIZ4,
		sLIZARD_LIZ6,
		sLORISOUNDS_DIE1,
		sLORISOUNDS_HURT0,
		sLORISOUNDS_HURT1,
		sLORISOUNDS_HURT2,
		sLORISOUNDS_HURT3,
		sLORISOUNDS_HURT4,
		sLORISOUNDS_HURT5,
		sLORISOUNDS_HURT6,
		sLORISOUNDS_HURT7,
		sLORISOUNDS_LORI1,
		sLORISOUNDS_LORI2,
		sLORISOUNDS_LORIBOOM,
		sLORISOUNDS_LORIFALL,
		sLORISOUNDS_LORIJUMP,
		sLORISOUNDS_LORIJUMP2,
		sLORISOUNDS_LORIJUMP3,
		sLORISOUNDS_LORIJUMP4,
		sLORISOUNDS_TOUCH,
		sLORISOUNDS_WEHOO,
		sMENUSOUNDS_SELECT0,
		sMENUSOUNDS_SELECT1,
		sMENUSOUNDS_SELECT2,
		sMENUSOUNDS_SELECT3,
		sMENUSOUNDS_SELECT4,
		sMENUSOUNDS_SELECT5,
		sMENUSOUNDS_SELECT6,
		sMENUSOUNDS_TYPE,
		sMENUSOUNDS_TYPEENTER,
		sMONKEY_SPLUT,
		sMONKEY_THROW,
		sMOTH_FLAPMOTH,
		sORANGE_BOEML,
		sORANGE_BOEMR,
		sORANGE_BUBBELSL,
		sORANGE_BUBBELSR,
		sORANGE_GLAS1L,
		sORANGE_GLAS1R,
		sORANGE_GLAS2L,
		sORANGE_GLAS2R,
		sORANGE_MERGE,
		sORANGE_SWEEP0L,
		sORANGE_SWEEP0R,
		sORANGE_SWEEP1L,
		sORANGE_SWEEP1R,
		sORANGE_SWEEP2L,
		sORANGE_SWEEP2R,
		sP2_CRUNCH,
		sP2_FART,
		sP2_FOEW1,
		sP2_FOEW4,
		sP2_FOEW5,
		sP2_FROG1,
		sP2_FROG2,
		sP2_FROG3,
		sP2_FROG4,
		sP2_FROG5,
		sP2_KISS4,
		sP2_OPEN,
		sP2_PINCH1,
		sP2_PINCH2,
		sP2_PLOPSEQ1,
		sP2_PLOPSEQ2,
		sP2_PLOPSEQ3,
		sP2_PLOPSEQ4,
		sP2_POEP,
		sP2_PTOEI,
		sP2_SPLOUT,
		sP2_SPLUT,
		sP2_THROW,
		sP2_TONG,
		sPICKUPS_BOING_CHECK,
		sPICKUPS_HELI2,
		sPICKUPS_STRETCH1A,
		sPINBALL_BELL,
		sPINBALL_FLIP1,
		sPINBALL_FLIP2,
		sPINBALL_FLIP3,
		sPINBALL_FLIP4,
		sQUEEN_LADYUP,
		sQUEEN_SCREAM,
		sRAPIER_GOSTDIE,
		sRAPIER_GOSTLOOP,
		sRAPIER_GOSTOOOH,
		sRAPIER_GOSTRIP,
		sRAPIER_HITCHAR,
		sROBOT_BIG1,
		sROBOT_BIG2,
		sROBOT_CAN1,
		sROBOT_CAN2,
		sROBOT_HYDRO,
		sROBOT_HYDRO2,
		sROBOT_HYDROFIL,
		sROBOT_HYDROPUF,
		sROBOT_IDLE1,
		sROBOT_IDLE2,
		sROBOT_JMPCAN1,
		sROBOT_JMPCAN10,
		sROBOT_JMPCAN2,
		sROBOT_JMPCAN3,
		sROBOT_JMPCAN4,
		sROBOT_JMPCAN5,
		sROBOT_JMPCAN6,
		sROBOT_JMPCAN7,
		sROBOT_JMPCAN8,
		sROBOT_JMPCAN9,
		sROBOT_METAL1,
		sROBOT_METAL2,
		sROBOT_METAL3,
		sROBOT_METAL4,
		sROBOT_METAL5,
		sROBOT_OPEN,
		sROBOT_OUT,
		sROBOT_POEP,
		sROBOT_POLE,
		sROBOT_SHOOT,
		sROBOT_STEP1,
		sROBOT_STEP2,
		sROBOT_STEP3,
		sROCK_ROCK1,
		sRUSH_RUSH,
		sSCIENCE_PLOPKAOS,
		sSKELETON_BONE1,
		sSKELETON_BONE2,
		sSKELETON_BONE3,
		sSKELETON_BONE5,
		sSKELETON_BONE6,
		sSKELETON_BONE7,
		sSMALTREE_FALL,
		sSMALTREE_GROUND,
		sSMALTREE_HEAD,
		sSONCSHIP_METAL1,
		sSONCSHIP_MISSILE2,
		sSONCSHIP_SCRAPE,
		sSONCSHIP_SHIPLOOP,
		sSONCSHIP_TARGETLOCK,
		sSPAZSOUNDS_AUTSCH1,
		sSPAZSOUNDS_AUTSCH2,
		sSPAZSOUNDS_BIRDSIT,
		sSPAZSOUNDS_BURP,
		sSPAZSOUNDS_CHIRP,
		sSPAZSOUNDS_EATBIRD,
		sSPAZSOUNDS_HAHAHA,
		sSPAZSOUNDS_HAHAHA2,
		sSPAZSOUNDS_HAPPY,
		sSPAZSOUNDS_HIHI,
		sSPAZSOUNDS_HOHOHO1,
		sSPAZSOUNDS_HOOO,
		sSPAZSOUNDS_KARATE7,
		sSPAZSOUNDS_KARATE8,
		sSPAZSOUNDS_OHOH,
		sSPAZSOUNDS_OOOH,
		sSPAZSOUNDS_WOOHOO,
		sSPAZSOUNDS_YAHOO,
		sSPAZSOUNDS_YAHOO2,
		sSPRING_BOING_DOWN,
		sSPRING_SPRING1,
		sSTEAM_STEAM,
		sSTONED_STONED,
		sSUCKER_FART,
		sSUCKER_PINCH1,
		sSUCKER_PINCH2,
		sSUCKER_PINCH3,
		sSUCKER_PLOPSEQ1,
		sSUCKER_PLOPSEQ2,
		sSUCKER_PLOPSEQ3,
		sSUCKER_PLOPSEQ4,
		sSUCKER_UP,
		sTUFBOSS_CATCH,
		sTUFBOSS_RELEASE,
		sTUFBOSS_SWING,
		sTURTLE_BITE3,
		sTURTLE_HIDE,
		sTURTLE_HITSHELL,
		sTURTLE_IDLE1,
		sTURTLE_IDLE2,
		sTURTLE_NECK,
		sTURTLE_SPK1TURT,
		sTURTLE_SPK2TURT,
		sTURTLE_SPK3TURT,
		sTURTLE_SPK4TURT,
		sTURTLE_TURN,
		sUTERUS_CRABCLOSE,
		sUTERUS_CRABOPEN2,
		sUTERUS_SCISSORS1,
		sUTERUS_SCISSORS2,
		sUTERUS_SCISSORS3,
		sUTERUS_SCISSORS4,
		sUTERUS_SCISSORS5,
		sUTERUS_SCISSORS6,
		sUTERUS_SCISSORS7,
		sUTERUS_SCISSORS8,
		sUTERUS_SCREAM1,
		sUTERUS_STEP1,
		sUTERUS_STEP2,
		sWIND_WIND2A,
		sWITCH_LAUGH,
		sWITCH_MAGIC,
		sXBILSY_BILLAPPEAR,
		sXBILSY_FINGERSNAP,
		sXBILSY_FIRE,
		sXBILSY_FIRESTART,
		sXBILSY_SCARY3,
		sXBILSY_THUNDER,
		sXBILSY_ZIP,
		sXLIZARD_LIZ1,
		sXLIZARD_LIZ2,
		sXLIZARD_LIZ4,
		sXLIZARD_LIZ6,
		sXTURTLE_BITE3,
		sXTURTLE_HIDE,
		sXTURTLE_HITSHELL,
		sXTURTLE_IDLE1,
		sXTURTLE_IDLE2,
		sXTURTLE_NECK,
		sXTURTLE_SPK1TURT,
		sXTURTLE_SPK2TURT,
		sXTURTLE_SPK3TURT,
		sXTURTLE_SPK4TURT,
		sXTURTLE_TURN,
		sZDOG_AGRESSIV,
		sZDOG_SNIF1,
		sZDOG_WAF1,
		sZDOG_WAF2,
		sZDOG_WAF3
	};

	enum state {
		sSTART,
		sSLEEP,
		sWAKE,
		sKILL,
		sDEACTIVATE,
		sWALK,
		sJUMP,
		sFIRE,
		sFLY,
		sBOUNCE,
		sEXPLODE,
		sROCKETFLY,
		sSTILL,
		sFLOAT,
		sHIT,
		sSPRING,
		sACTION,
		sDONE,
		sPUSH,
		sFALL,
		sFLOATFALL,
		sCIRCLE,
		sATTACK,
		sFREEZE,
		sFADEIN,
		sFADEOUT,
		sHIDE,
		sTURN,
		sIDLE,
		sEXTRA,
		sSTOP,
		sWAIT,
		sLAND,
		sDELAYEDSTART,
		sROTATE,
		sDUCK
	};

	enum tbgMode {
		tbgModeWARPHORIZON,
		tbgModeTUNNEL,
		tbgModeMENU,
		tbgModeTILEMENU,
		tbgModeWAVE,
		tbgModeCYLINDER,
		tbgModeREFLECTION
	};


	struct jjTEXTAPPEARANCE {
		enum align_ {
			align_DEFAULT,
			align_LEFT,
			align_CENTER,
			align_RIGHT
		};

		enum ch_ {
			ch_HIDE,
			ch_DISPLAY,
			ch_SPECIAL
		};

		int xAmp;
		int yAmp;
		int spacing;
		bool monospace;
		bool skipInitialHash;

		ch_ at = ch_HIDE;
		ch_ caret = ch_HIDE;
		ch_ hash = ch_HIDE;
		ch_ newline = ch_HIDE;
		ch_ pipe = ch_HIDE;
		ch_ section = ch_HIDE;
		ch_ tilde = ch_HIDE;
		align_ align = align_DEFAULT;

		static jjTEXTAPPEARANCE constructor() {
			noop(); return { };
		}
		static jjTEXTAPPEARANCE constructorMode(uint32_t mode) {
			noop(); return { };
		}

		jjTEXTAPPEARANCE& operator=(uint32_t other) {
			noop(); return *this;
		}
	};

	struct jjPALCOLOR {
		uint8_t red;
		uint8_t green;
		uint8_t blue;

		static jjPALCOLOR Create() {
			noop(); return { };
		}
		static jjPALCOLOR CreateFromRgb(uint8_t red, uint8_t green, uint8_t blue) {
			noop(); return { red, green, blue };
		}

		uint8_t getHue() {
			noop(); return 0;
		}
		uint8_t getSat() {
			noop(); return 0;
		}
		uint8_t getLight() {
			noop(); return 0;
		}

		void swizzle(uint32_t redc, uint32_t greenc, uint32_t bluec) {
			noop();

			uint8_t r = red;
			uint8_t g = green;
			uint8_t b = blue;

			switch (redc) {
				case 1: red = g; break;
				case 2: red = b; break;
			}
			switch (greenc) {
				case 0: green = r; break;
				case 2: green = b; break;
			}
			switch (bluec) {
				case 0: blue = r; break;
				case 1: blue = g; break;
			}
		}
		void setHSL(int hue, uint8_t sat, uint8_t light) {
			noop();
		}

		jjPALCOLOR& operator=(const jjPALCOLOR& other) {
			noop();
			
			red = other.red;
			green = other.green;
			blue = other.blue;
			return *this;
		}
		bool operator==(const jjPALCOLOR& other) {
			noop(); return (red == other.red && green == other.green && blue == other.blue);
		}
	};

	class jjPAL
	{
	public:
		jjPAL() : _refCount(1) {
			noop();
		}
		~jjPAL() {
			noop();
		}

		static jjPAL* Create(jjPAL* self) {
			noop();
			return new(self) jjPAL();
		}

		void AddRef()
		{
			_refCount++;
		}

		void Release()
		{
			if (--_refCount == 0) {
				this->~jjPAL();
				asFreeMem(this);
			}
		}

		void reset() {
			noop();
		}
		void apply() {
			noop();
		}
		bool load(const String& filename) {
			noop();
			return false;
		}
		void fill(uint8_t red, uint8_t green, uint8_t blue, float opacity) {
			noop();
		}
		void fillTint(uint8_t red, uint8_t green, uint8_t blue, uint8_t start, uint8_t length, float opacity) {
			noop();
		}
		void fillFromColor(jjPALCOLOR color, float opacity) {
			noop();
		}
		void fillTintFromColor(jjPALCOLOR color, uint8_t start, uint8_t length, float opacity) {
			noop();
		}
		void gradient(uint8_t red1, uint8_t green1, uint8_t blue1, uint8_t red2, uint8_t green2, uint8_t blue2, uint8_t start, uint8_t length, float opacity, bool inclusive) {
			noop();
		}
		void gradientFromColor(jjPALCOLOR color1, jjPALCOLOR color2, uint8_t start, uint8_t length, float opacity, bool inclusive) {
			noop();
		}
		void copyFrom(uint8_t start, uint8_t length, uint8_t start2, const jjPAL& source, float opacity) {
			noop();
		}
		uint8_t findNearestColor(jjPALCOLOR color) {
			noop();
			return 0;
		}

	private:
		int _refCount;
	};

	jjPAL jjPalette;
	jjPAL jjBackupPalette;

	class jjSTREAM
	{
	public:
		jjSTREAM() : _refCount(1) {
			noop();
		}
		~jjSTREAM() {
			noop();
		}

		static jjSTREAM* Create() {
			noop();

			auto ctx = asGetActiveContext();
			auto owner = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(ScriptLoader::EngineToOwner));

			void* mem = asAllocMem(sizeof(jjSTREAM));
			return new(mem) jjSTREAM();
		}
		static jjSTREAM* CreateFromFile(const String& filename) {
			noop();

			auto ctx = asGetActiveContext();
			auto owner = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(ScriptLoader::EngineToOwner));

			void* mem = asAllocMem(sizeof(jjSTREAM));
			return new(mem) jjSTREAM();
		}

		void AddRef()
		{
			_refCount++;
		}

		void Release()
		{
			if (--_refCount == 0) {
				this->~jjSTREAM();
				asFreeMem(this);
			}
		}

		// Assignment operator
		jjSTREAM& operator=(const jjSTREAM& o)
		{
			// Copy only the content, not the script proxy class
			//_value = o._value;
			return *this;
		}

		uint32_t getSize() const {
			noop();
			return 0;
		}

		bool isEmpty() const {
			noop();
			return false;
		}

		bool save(const String& tilename) const {
			noop();
			return false;
		}

		void clear() {
			noop();
		}

		bool discard(uint32_t count) {
			return 0;
		}

		bool write(const String& value) {
			noop();
			return false;
		}
		bool write(const jjSTREAM& value) {
			noop();
			return false;
		}
		bool get(String& value, uint32_t count) {
			noop();
			return false;
		}
		bool get(jjSTREAM& value, uint32_t count) {
			noop();
			return false;
		}
		bool getLine(String& value, const String& delim) {
			noop();
			return false;
		}

		bool push(bool value) {
			noop();
			return false;
		}
		bool push(uint8_t value) {
			noop();
			return false;
		}
		bool push(int8_t value) {
			noop();
			return false;
		}
		bool push(uint16_t value) {
			noop();
			return false;
		}
		bool push(int16_t value) {
			noop();
			return false;
		}
		bool push(uint32_t value) {
			noop();
			return false;
		}
		bool push(int32_t value) {
			noop();
			return false;
		}
		bool push(uint64_t value) {
			noop();
			return false;
		}
		bool push(int64_t value) {
			noop();
			return false;
		}
		bool push(float value) {
			noop();
			return false;
		}
		bool push(double value) {
			noop();
			return false;
		}
		bool push(const String& value) {
			noop();
			return false;
		}
		bool push(const jjSTREAM& value) {
			noop();
			return false;
		}

		bool pop(bool& value) {
			noop();
			return false;
		}
		bool pop(uint8_t& value) {
			noop();
			return false;
		}
		bool pop(int8_t& value) {
			noop();
			return false;
		}
		bool pop(uint16_t& value) {
			noop();
			return false;
		}
		bool pop(int16_t& value) {
			noop();
			return false;
		}
		bool pop(uint32_t& value) {
			noop();
			return false;
		}
		bool pop(int32_t& value) {
			noop();
			return false;
		}
		bool pop(uint64_t& value) {
			noop();
			return false;
		}
		bool pop(int64_t& value) {
			noop();
			return false;
		}
		bool pop(float& value) {
			noop();
			return false;
		}
		bool pop(double& value) {
			noop();
			return false;
		}
		bool pop(String& value) {
			noop();
			return false;
		}
		bool pop(jjSTREAM& value) {
			noop();
			return false;
		}

	private:
		int _refCount;
	};

	struct jjBEHAVIOR
	{
		static jjBEHAVIOR* Create(jjBEHAVIOR* self) {
			noop();
			return new(self) jjBEHAVIOR();
		}
		static jjBEHAVIOR* CreateFromBehavior(uint32_t behavior, jjBEHAVIOR* self) {
			noop();
			return new(self) jjBEHAVIOR();
		}
		static void Destroy(jjBEHAVIOR* self) {
			noop();
		}

		jjBEHAVIOR& operator=(const jjBEHAVIOR& other) {
			noop();
			return *this;
		}
		jjBEHAVIOR& operator=(uint32_t other) {
			noop();
			return *this;
		}
		jjBEHAVIOR& operator=(asIScriptFunction* other) {
			noop();
			return *this;
		}
		jjBEHAVIOR& operator=(asIScriptObject* other) {
			noop();
			return *this;
		}
		bool operator==(const jjBEHAVIOR& other) const {
			noop();
			return false;
		}
		bool operator==(uint32_t other) const {
			noop();
			return false;
		}
		bool operator==(const asIScriptFunction* other) const {
			noop();
			return false;
		}
		operator uint32_t() {
			noop();
			return 0;
		}
		operator asIScriptFunction* () {
			noop();
			return nullptr;
		}
		operator asIScriptObject* () {
			noop();
			return nullptr;
		}
	};

	class jjANIMFRAME
	{
	public:
		jjANIMFRAME() : _refCount(1) {
			noop();
		}
		~jjANIMFRAME() {
			noop();
		}

		void AddRef()
		{
			_refCount++;
		}

		void Release()
		{
			if (--_refCount == 0) {
				this->~jjANIMFRAME();
				asFreeMem(this);
			}
		}

		// Assignment operator
		jjANIMFRAME& operator=(const jjANIMFRAME& o)
		{
			// Copy only the content, not the script proxy class
			//_value = o._value;
			return *this;
		}

		static jjANIMFRAME* get_jjAnimFrames(uint32_t index) {
			noop();

			auto ctx = asGetActiveContext();
			auto owner = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(ScriptLoader::EngineToOwner));

			void* mem = asAllocMem(sizeof(jjANIMFRAME));
			return new(mem) jjANIMFRAME();
		}

		int16_t hotSpotX = 0;
		int16_t hotSpotY = 0;
		int16_t coldSpotX = 0;
		int16_t coldSpotY = 0;
		int16_t gunSpotX = 0;
		int16_t gunSpotY = 0;
		int16_t width = 0;
		int16_t height = 0;

		bool get_transparent() const {
			noop();
			return false;
		}
		bool set_transparent(bool value) const {
			noop();
			return false;
		}
		bool doesCollide(int32_t xPos, int32_t yPos, int32_t direction, const jjANIMFRAME* frame2, int32_t xPos2, int32_t yPos2, int32_t direction2, bool always) const {
			noop();
			return false;
		}

	private:
		int _refCount;
	};

	class jjANIMATION
	{
	public:
		jjANIMATION(uint32_t index) : _refCount(1), _index(index) {
			noop();
		}
		~jjANIMATION() {
			noop();
		}

		void AddRef()
		{
			_refCount++;
		}

		void Release()
		{
			if (--_refCount == 0) {
				this->~jjANIMATION();
				asFreeMem(this);
			}
		}

		// Assignment operator
		jjANIMATION& operator=(const jjANIMATION& o)
		{
			// Copy only the content, not the script proxy class
			//_value = o._value;
			return *this;
		}

		bool save(const String& filename, const jjPAL& palette) const {
			noop();
			return false;
		}
		bool load(const String& filename, int32_t hotSpotX, int32_t hotSpotY, int32_t coldSpotYOffset, int32_t firstFrameToOverwrite) {
			noop();
			return false;
		}

		static jjANIMATION* get_jjAnimations(uint32_t index) {
			noop();

			auto ctx = asGetActiveContext();
			auto owner = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(ScriptLoader::EngineToOwner));

			void* mem = asAllocMem(sizeof(jjANIMATION));
			return new(mem) jjANIMATION(index);
		}

		uint16_t frameCount = 0;
		int16_t fps = 0;

		uint32_t get_firstFrame() const {
			noop();
			return 0;
		}
		uint32_t set_firstFrame(uint32_t index) const {
			noop();
			return 0;
		}

		uint32_t getAnimFirstFrame() {
			noop();
			return 0;
		}

	private:
		int _refCount;
		uint32_t _index;
	};

	class jjANIMSET
	{
	public:
		jjANIMSET(uint32_t index) : _refCount(1), _index(index) {
			noop();
		}
		~jjANIMSET() {
			noop();
		}

		void AddRef()
		{
			_refCount++;
		}

		void Release()
		{
			if (--_refCount == 0) {
				this->~jjANIMSET();
				asFreeMem(this);
			}
		}

		static jjANIMSET* get_jjAnimSets(uint32_t index) {
			noop();

			auto ctx = asGetActiveContext();
			auto owner = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(ScriptLoader::EngineToOwner));

			void* mem = asAllocMem(sizeof(jjANIMSET));
			return new(mem) jjANIMSET(index);
		}

		uint32_t convertAnimSetToUint() {
			noop();
			return _index;
		}

		jjANIMSET* load(uint32_t fileSetID, const String& filename, int32_t firstAnimToOverwrite, int32_t firstFrameToOverwrite) {
			noop();
			return this;
		}
		jjANIMSET* allocate(const CScriptArray& frameCounts) {
			noop();
			return this;
		}

	private:
		int _refCount;
		uint32_t _index;
	};

	struct jjCANVAS {

		void DrawPixel(int32_t xPixel, int32_t yPixel, uint8_t color, uint32_t mode, uint8_t param) {
			noop();
		}
		void DrawRectangle(int32_t xPixel, int32_t yPixel, int32_t width, int32_t height, uint8_t color, uint32_t mode, uint8_t param) {
			noop();
		}
		void DrawSprite(int32_t xPixel, int32_t yPixel, int32_t setID, uint8_t animation, uint8_t frame, int8_t direction, uint32_t mode, uint8_t param) {
			noop();
		}
		void DrawCurFrameSprite(int32_t xPixel, int32_t yPixel, uint32_t sprite, int8_t direction, uint32_t mode, uint8_t param) {
			noop();
		}
		void DrawResizedSprite(int32_t xPixel, int32_t yPixel, int32_t setID, uint8_t animation, uint8_t frame, float xScale, float yScale, uint32_t mode, uint8_t param) {
			noop();
		}
		void DrawResizedCurFrameSprite(int32_t xPixel, int32_t yPixel, uint32_t sprite, float xScale, float yScale, uint32_t mode, uint8_t param) {
			noop();
		}
		void DrawTransformedSprite(int32_t xPixel, int32_t yPixel, int32_t setID, uint8_t animation, uint8_t frame, int32_t angle, float xScale, float yScale, uint32_t mode, uint8_t param) {
			noop();
		}
		void DrawTransformedCurFrameSprite(int32_t xPixel, int32_t yPixel, uint32_t sprite, int32_t angle, float xScale, float yScale, uint32_t mode, uint8_t param) {
			noop();
		}
		void DrawSwingingVine(int32_t xPixel, int32_t yPixel, uint32_t sprite, int32_t length, int32_t curvature, uint32_t mode, uint8_t param) {
			noop();
		}

		void ExternalDrawTile(int32_t xPixel, int32_t yPixel, uint16_t tile, uint32_t tileQuadrant) {
			noop();
		}
		void DrawTextBasicSize(int32_t xPixel, int32_t yPixel, const String& text, uint32_t size, uint32_t mode, uint8_t param) {
			noop();
		}
		void DrawTextExtSize(int32_t xPixel, int32_t yPixel, const String& text, uint32_t size, const jjTEXTAPPEARANCE& appearance, uint8_t param1, uint32_t mode, uint8_t param) {
			noop();
		}

		void drawString(int32_t xPixel, int32_t yPixel, const String& text, const jjANIMATION& animation, uint32_t mode, uint8_t param) {
			noop();
		}

		void drawStringEx(int32_t xPixel, int32_t yPixel, const String& text, const jjANIMATION& animation, const jjTEXTAPPEARANCE& appearance, uint8_t param1, uint32_t spriteMode, uint8_t param2) {
			noop();
		}

		static void jjDrawString(float xPixel, float yPixel, const String& text, const jjANIMATION& animation, uint32_t mode, uint8_t param, int8_t layerZ, uint8_t layerXY, int8_t playerID) {
			noop();
		}

		static void jjDrawStringEx(float xPixel, float yPixel, const String& text, const jjANIMATION& animation, const jjTEXTAPPEARANCE& appearance, uint8_t param1, uint32_t spriteMode, uint8_t param2, int8_t layerZ, uint8_t layerXY, int8_t playerID) {
			noop();
		}

		static int jjGetStringWidth(const String& text, const jjANIMATION& animation, const jjTEXTAPPEARANCE& style) {
			noop();
			return 0;
		}
	};

	class jjOBJ;
	class jjPLAYER;

	using jjVOIDFUNCOBJ = void(*)(jjOBJ* obj);

	class jjOBJ
	{
	public:
		jjOBJ() : _refCount(1) {
			noop();
		}
		~jjOBJ() {
			noop();
		}

		void AddRef()
		{
			_refCount++;
		}

		void Release()
		{
			if (--_refCount == 0) {
				this->~jjOBJ();
				asFreeMem(this);
			}
		}

		bool get_isActive() const {
			noop();
			return true;
		}
		uint32_t get_lightType() const {
			noop();
			return 0;
		}
		uint32_t set_lightType(uint32_t value) const {
			noop();
			return 0;
		}

		jjOBJ* objectHit(jjOBJ* target, uint32_t playerHandling) {
			noop();
			return nullptr;
		}
		void blast(int32_t maxDistance, bool blastObjects) {
			noop();
		}

		jjBEHAVIOR behavior;

		void behave1(uint32_t behavior, bool draw) {
			noop();
		}
		void behave2(jjBEHAVIOR behavior, bool draw) {
			noop();
		}
		void behave3(jjVOIDFUNCOBJ behavior, bool draw) {
			noop();
		}

		static int32_t jjAddObject(uint8_t eventID, float xPixel, float yPixel, uint16_t creatorID, uint32_t creatorType, uint32_t behavior) {
			noop();
			return 0;
		}
		static int32_t jjAddObjectEx(uint8_t eventID, float xPixel, float yPixel, uint16_t creatorID, uint32_t creatorType, jjVOIDFUNCOBJ behavior) {
			noop();
			return 0;
		}

		static void jjDeleteObject(int32_t objectID) {
			noop();
		}
		static void jjKillObject(int32_t objectID) {
			noop();
		}

		float xOrg = 0;
		float yOrg = 0;
		float xPos = 0;
		float yPos = 0;
		float xSpeed = 0;
		float ySpeed = 0;
		float xAcc = 0;
		float yAcc = 0;
		int32_t counter = 0;
		uint32_t curFrame = 0;

		uint32_t determineCurFrame(bool change) {
			noop(); return 0;
		}

		int32_t age = 0;
		int32_t creator = 0;

		uint16_t get_creatorID() const {
			noop(); return 0;
		}
		uint16_t set_creatorID(uint16_t value) const {
			noop(); return 0;
		}
		uint32_t get_creatorType() const {
			noop(); return 0;
		}
		uint32_t set_creatorType(uint32_t value) const {
			noop(); return 0;
		}

		int16_t curAnim = 0;

		int16_t determineCurAnim(uint8_t setID, uint8_t animation, bool change) {
			noop(); return 0;
		}

		uint16_t killAnim = 0;
		uint8_t freeze = 0;
		uint8_t lightType = 0;
		int8_t frameID = 0;
		int8_t noHit = 0;

		uint32_t get_bulletHandling() {
			noop(); return 0;
		}
		uint32_t set_bulletHandling(uint32_t value) {
			noop(); return 0;
		}
		bool get_ricochet() {
			noop(); return 0;
		}
		bool set_ricochet(bool value) {
			noop(); return 0;
		}
		bool get_freezable() {
			noop(); return 0;
		}
		bool set_freezable(bool value) {
			noop(); return 0;
		}
		bool get_blastable() {
			noop(); return 0;
		}
		bool set_blastable(bool value) {
			noop(); return 0;
		}

		int8_t energy = 0;
		int8_t light = 0;
		uint8_t objType = 0;

		uint32_t get_playerHandling() {
			noop(); return false;
		}
		uint32_t set_playerHandling(uint32_t value) {
			noop(); return false;
		}
		bool get_isTarget() {
			noop(); return false;
		}
		bool set_isTarget(bool value) {
			noop(); return false;
		}
		bool get_triggersTNT() {
			noop(); return false;
		}
		bool set_triggersTNT(bool value) {
			noop(); return false;
		}
		bool get_deactivates() {
			noop(); return false;
		}
		bool set_deactivates(bool value) {
			noop(); return false;
		}
		bool get_scriptedCollisions() {
			noop(); return false;
		}
		bool set_scriptedCollisions(bool value) {
			noop(); return false;
		}

		int8_t state = 0;
		uint16_t points = 0;
		uint8_t eventID = 0;
		int8_t direction = 0;
		uint8_t justHit = 0;
		int8_t oldState = 0;
		int32_t animSpeed = 0;
		int32_t special = 0;

		int32_t get_var(uint8_t x) {
			noop(); return 0;
		}
		int32_t set_var(uint8_t x, int32_t value) {
			noop(); return 0;
		}

		uint8_t doesHurt = 0;
		uint8_t counterEnd = 0;
		int16_t objectID = 0;

		int32_t draw() {
			noop();
			return 0;
		}
		int32_t beSolid(bool shouldCheckForStompingLocalPlayers) {
			noop();
			return 0;
		}
		void bePlatform(float xOld, float yOld, int32_t width, int32_t height) {
			noop();
		}
		void clearPlatform() {
			noop();
		}
		void putOnGround(bool precise) {
			noop();
		}
		bool ricochet() {
			noop();
			return false;
		}
		int32_t unfreeze(int32_t style) {
			noop();
			return 0;
		}
		void deleteObject() {
			noop();
		}
		void deactivate() {
			noop();
		}
		void pathMovement() {
			noop();
		}
		int32_t fireBullet(uint8_t eventID) {
			noop();
			return 0;
		}
		void particlePixelExplosion(int32_t style) {
			noop();
		}
		void grantPickup(jjPLAYER* player, int32_t frequency) {
			noop();
		}

		int findNearestPlayer(int maxDistance) const {
			noop();
			return 0;
		}
		int findNearestPlayerEx(int maxDistance, int& foundDistance) const {
			noop();
			return 0;
		}

		bool doesCollide(const jjOBJ* object, bool always) const {
			noop();
			return false;
		}
		bool doesCollidePlayer(const jjPLAYER* object, bool always) const {
			noop();
			return false;
		}

	private:
		int _refCount;
	};

	jjOBJ* get_jjObjects(int index)
	{
		noop();
		auto ctx = asGetActiveContext();
		auto owner = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(ScriptLoader::EngineToOwner));

		void* mem = asAllocMem(sizeof(jjOBJ));
		return new(mem) jjOBJ();
	}

	jjOBJ* get_jjObjectPresets(int8_t id)
	{
		noop();
		auto ctx = asGetActiveContext();
		auto owner = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(ScriptLoader::EngineToOwner));

		void* mem = asAllocMem(sizeof(jjOBJ));
		return new(mem) jjOBJ();
	}

	int jjObjectCount = 0;
	int jjObjectMax = 0;

	class jjPLAYER
	{
	public:
		jjPLAYER(LevelScriptLoader* levelScripts, int playerIndex) : _levelScriptLoader(levelScripts), _refCount(1) {
			noop();
			auto& players = levelScripts->GetPlayers();
			_player = (playerIndex < players.size() ? players[playerIndex] : nullptr);
		}
		jjPLAYER(LevelScriptLoader* levelScripts, Actors::Player* player) : _levelScriptLoader(levelScripts), _refCount(1), _player(player) {
			noop();
		}
		~jjPLAYER() {
			noop();
		}

		void AddRef()
		{
			_refCount++;
		}

		void Release()
		{
			if (--_refCount == 0) {
				this->~jjPLAYER();
				asFreeMem(this);
			}
		}

		// Assignment operator
		jjPLAYER& operator=(const jjPLAYER& o)
		{
			// Copy only the content, not the script proxy class
			//_value = o._value;
			return *this;
		}

		int32_t score = 0;
		int32_t lastScoreDisplay = 0;

		int32_t setScore(int32_t value) {
			noop(); return 0;
		}

		float xPos = 0.0f;
		float yPos = 0.0f;
		float xAcc = 0.0f;
		float yAcc = 0.0f;
		float xOrg = 0.0f;
		float yOrg = 0.0f;

		float get_xSpeed() {
			noop();
			return _player->_speed.X;
		}
		float set_xSpeed(float value) {
			noop();
			_player->_speed.X = value;
			return value;
		}
		float get_ySpeed() {
			noop();
			return _player->_speed.Y;
		}
		float set_ySpeed(float value) {
			noop();
			_player->_speed.Y = value;
			return value;
		}

		float jumpStrength = 0.0f;
		int8_t frozen = 0;

		void freeze(bool frozen) {
			noop();
			if (frozen) {
				_player->_frozenTimeLeft = 180.0f;
				_player->_renderer.AnimPaused = true;
			} else {
				_player->_frozenTimeLeft = std::min(1.0f, _player->_frozenTimeLeft);
			}
		}
		int32_t get_currTile() {
			noop();
			return 0;
		}
		bool startSugarRush(int32_t time) {
			noop();
			// TODO: if boss active, return false
			_player->ActivateSugarRush((float)time * 60.0f / 70.0f);
			return true;
		}
		int8_t get_health() const {
			noop();
			return (int8_t)_player->_health;
		}
		int8_t set_health(int8_t value) {
			noop();
			_player->SetHealth(value);
			return value;
		}

		int32_t warpID = 0;
		int32_t fastfire = 0;

		int8_t get_currWeapon() const {
			noop();
			return (int8_t)_player->_currentWeapon;
		}
		int8_t set_currWeapon(int8_t value) {
			noop();
			if (value < 0 || value >= (int8_t)WeaponType::Count) {
				return (int8_t)_player->_currentWeapon;
			}
			_player->_currentWeapon = (WeaponType)value;
			return value;
		}

		int32_t lives = 1;
		int32_t invincibility = 0;
		int32_t blink = 0;

		int32_t extendInvincibility(int32_t duration) {
			noop();
			return 0;
		}

		int32_t food = 0;
		int32_t coins = 0;

		bool testForCoins(int32_t numberOfCoins) {
			noop();
			if (numberOfCoins > _player->_coins) {
				return false;
			}
			_player->AddCoins(-numberOfCoins);
			return true;
		}
		int32_t get_gems(uint32_t type) const {
			noop();
			return 0;
		}
		int32_t set_gems(uint32_t type, int32_t value) {
			noop();
			return 0;
		}
		bool testForGems(int32_t numberOfGems, uint32_t type) {
			noop();
			return false;
		}

		int32_t shieldType = 0;
		int32_t shieldTime = 0;
		int32_t rolling = 0;
		int32_t bossNumber = 0;
		int32_t boss = 0;
		bool bossActive = false;
		int8_t direction = 0;
		int32_t platform = 0;
		int32_t flag = 0;
		int32_t clientID = 0;
		int8_t playerID = 0;
		int32_t localPlayerID = 0;
		bool team = false;
		bool run = false;
		int32_t specialJump = 0;

		int32_t get_stoned() {
			noop();
			return (int32_t)(_player->_dizzyTime * 70.0f / 60.0f);
		}
		int32_t set_stoned(int32_t value) {
			noop();
			_player->SetDizzyTime(value * 60.0f / 70.0f);
			return value;
		}

		int32_t buttstomp = 0;
		int32_t helicopter = 0;
		int32_t helicopterElapsed = 0;
		int32_t specialMove = 0;
		int32_t idle = 0;

		void suckerTube(int32_t xSpeed, int32_t ySpeed, bool center, bool noclip, bool trigSample) {
			noop();
		}
		void poleSpin(float xSpeed, float ySpeed, uint32_t delay) {
			noop();
		}
		void spring(float xSpeed, float ySpeed, bool keepZeroSpeeds, bool sample) {
			noop();
		}

		bool isLocal = true;
		bool isActive = true;

		bool get_isConnecting() const {
			noop();
			return false;
		}
		bool get_isIdle() const {
			noop();
			return false;
		}
		bool get_isOut() const {
			noop();
			return false;
		}
		bool get_isSpectating() const {
			noop();
			return false;
		}
		bool get_isInGame() const {
			noop();
			return true;
		}

		String get_name() const {
			noop();
			return { };
		}
		String get_nameUnformatted() const {
			noop();
			return { };
		}
		bool setName(const String& name) {
			noop();
			return false;
		}
		int8_t get_light() const {
			noop();
			return 0;
		}
		int8_t set_light(int8_t value) {
			noop();
			return 0;
		}
		uint32_t get_fur() const {
			noop();
			return 0;
		}
		uint32_t set_fur(uint32_t value) {
			noop();
			return 0;
		}

		bool get_noFire() const {
			noop();
			return !_player->_weaponAllowed;
		}
		bool set_noFire(bool value) {
			noop();
			_player->_weaponAllowed = !value;
			return value;
		}
		bool get_antiGrav() const {
			noop();
			return false;
		}
		bool set_antiGrav(bool value) {
			noop();
			return false;
		}
		bool get_invisibility() const {
			noop();
			return _player->GetState(Actors::ActorState::IsInvulnerable);
		}
		bool set_invisibility(bool value) {
			noop();
			return false;
		}
		bool get_noclipMode() const {
			noop();
			return false;
		}
		bool set_noclipMode(bool value) {
			noop();
			return false;
		}
		uint8_t get_lighting() const {
			noop();
			return (uint8_t)(_levelScriptLoader->_levelHandler->GetAmbientLight() * 64.0f);
		}
		uint8_t set_lighting(uint8_t value) {
			noop();
			_levelScriptLoader->_levelHandler->SetAmbientLight(value / 64.0f);
			return value;
		}
		uint8_t resetLight() {
			noop();
			return 0;
		}

		bool get_playerKeyLeftPressed() {
			noop(); return false;
		}
		bool get_playerKeyRightPressed() {
			noop(); return false;
		}
		bool get_playerKeyUpPressed() {
			noop(); return false;
		}
		bool get_playerKeyDownPressed() {
			noop(); return false;
		}
		bool get_playerKeyFirePressed() {
			noop(); return false;
		}
		bool get_playerKeySelectPressed() {
			noop(); return false;
		}
		bool get_playerKeyJumpPressed() {
			noop(); return false;
		}
		bool get_playerKeyRunPressed() {
			noop(); return false;
		}
		void set_playerKeyLeftPressed(bool value) {
			noop();
		}
		void set_playerKeyRightPressed(bool value) {
			noop();
		}
		void set_playerKeyUpPressed(bool value) {
			noop();
		}
		void set_playerKeyDownPressed(bool value) {
			noop();
		}
		void set_playerKeyFirePressed(bool value) {
			noop();
		}
		void set_playerKeySelectPressed(bool value) {
			noop();
		}
		void set_playerKeyJumpPressed(bool value) {
			noop();
		}
		void set_playerKeyRunPressed(bool value) {
			noop();
		}

		bool get_powerup(uint8_t index) {
			noop();
			if (index < 0 || index >= (int8_t)WeaponType::Count) {
				return 0;
			}
			return (_player->_weaponUpgrades[index] & 0x01) == 0x01;
		}
		bool set_powerup(uint8_t index, bool value) {
			noop();
			if (index < 0 || index >= (int8_t)WeaponType::Count) {
				return 0;
			}
			_player->_weaponUpgrades[index] = (value ? 0x01 : 0x00);
			return value;
		}
		int32_t get_ammo(uint8_t index) const {
			noop();
			if (index < 0 || index >= (int8_t)WeaponType::Count) {
				return 0;
			}
			return _player->_weaponAmmo[index];
		}
		int32_t set_ammo(uint8_t index, int32_t value) {
			noop();
			if (index < 0 || index >= (int8_t)WeaponType::Count) {
				return 0;
			}
			_player->_weaponAmmo[index] = value * 256;
			return value;
		}

		bool offsetPosition(int32_t xPixels, int32_t yPixels) {
			noop();
			
			Vector2f pos = _player->GetPos();
			_player->WarpToPosition(Vector2f(pos.X + xPixels, pos.Y + yPixels), true);
			return true;
		}
		bool warpToTile(int32_t xTile, int32_t yTile, bool fast) {
			noop();

			_player->WarpToPosition(Vector2f(xTile * TileSet::DefaultTileSize + Tiles::TileSet::DefaultTileSize / 2, yTile * TileSet::DefaultTileSize + Tiles::TileSet::DefaultTileSize / 2), fast);
			return true;
		}
		bool warpToID(uint8_t warpID, bool fast) {
			noop();

			auto events = _levelScriptLoader->_levelHandler->EventMap();
			Vector2f c = events->GetWarpTarget(warpID);
			if (c.X >= 0.0f && c.Y >= 0.0f) {
				_player->WarpToPosition(c, fast);
				return true;
			}
			return false;
		}

		uint32_t morph(bool rabbitsOnly, bool morphEffect) {
			noop();
			return 0;
		}
		uint32_t morphTo(uint32_t charNew, bool morphEffect) {
			noop();
			// TODO: morphEffect
			_player->MorphTo((PlayerType)charNew);
			return (uint32_t)_player->_playerType;
		}
		uint32_t revertMorph(bool morphEffect) {
			noop();
			// TODO: morphEffect
			_player->MorphRevert();
			return (uint32_t)_player->_playerType;
		}
		uint32_t get_charCurr() const {
			noop();
			return (uint32_t)_player->_playerType;
		}

		uint32_t charOrig = 0;

		void kill() {
			noop();
			_player->DecreaseHealth(INT32_MAX);
		}
		bool hurt(int8_t damage, bool forceHurt, jjPLAYER* attacker) {
			noop();

			// TODO: forceHurt and return value
			_player->TakeDamage(damage);
			return false;
		}

		uint32_t get_timerState() const {
			noop();
			return 0;
		}
		bool get_timerPersists() const {
			noop();
			return false;
		}
		bool set_timerPersists(bool value) {
			noop();
			return false;
		}
		uint32_t timerStart(int32_t ticks, bool startPaused) {
			noop();
			return 0;
		}
		uint32_t timerPause() {
			noop();
			return 0;
		}
		uint32_t timerResume() {
			noop();
			return 0;
		}
		uint32_t timerStop() {
			noop();
			return 0;
		}
		int32_t get_timerTime() const {
			noop();
			return 0;
		}
		int32_t set_timerTime(int32_t value) {
			noop();
			return 0;
		}
		void timerFunction(const String& functionName) {
			noop();
		}
		void timerFunctionPtr(void* function) {
			noop();
		}
		void timerFunctionFuncPtr(void* function) {
			noop();
		}

		bool activateBoss(bool activate) {
			noop();
			
			// TODO: activate
			_levelScriptLoader->_levelHandler->BroadcastTriggeredEvent(_player, EventType::AreaActivateBoss, nullptr);
			return true;
		}
		bool limitXScroll(uint16_t left, uint16_t width) {
			noop();
				
			_levelScriptLoader->_levelHandler->LimitCameraView(left * Tiles::TileSet::DefaultTileSize, width * Tiles::TileSet::DefaultTileSize);
			return true;
		}
		void cameraFreezeFF(float xPixel, float yPixel, bool centered, bool instant) {
			noop();
		}
		void cameraFreezeBF(bool xUnfreeze, float yPixel, bool centered, bool instant) {
			noop();
		}
		void cameraFreezeFB(float xPixel, bool yUnfreeze, bool centered, bool instant) {
			noop();
		}
		void cameraFreezeBB(bool xUnfreeze, bool yUnfreeze, bool centered, bool instant) {
			noop();
		}
		void cameraUnfreeze(bool instant) {
			noop();
		}
		void showText(const String& text, uint32_t size) {
			noop();

			// TODO: size
			// Input string must be recoded in Legacy context
			auto recodedText = Compatibility::JJ2Strings::RecodeString(text);
			_levelScriptLoader->_levelHandler->ShowLevelText(recodedText);
		}
		void showTextByID(uint32_t textID, uint32_t offset, uint32_t size) {
			noop();

			// TODO: size
			auto text = _levelScriptLoader->_levelHandler->GetLevelText(textID, offset, '|');
			_levelScriptLoader->_levelHandler->ShowLevelText(text);
		}

		uint32_t get_fly() const {
			noop();
			return 0;
		}
		uint32_t set_fly(uint32_t value) {
			noop();
			return 0;
		}

		int32_t fireBulletDirection(uint8_t gun, bool depleteAmmo, bool requireAmmo, uint32_t direction) {
			noop();
			return 0;
		}
		int32_t fireBulletAngle(uint8_t gun, bool depleteAmmo, bool requireAmmo, float angle) {
			noop();
			return 0;
		}

		int32_t subscreenX = 0;
		int32_t subscreenY = 0;

		float get_cameraX() const {
			noop();
			return 0.0f;
		}
		float get_cameraY() const {
			noop();
			return 0.0f;
		}
		int32_t get_deaths() const {
			noop();
			return 0;
		}

		bool get_isJailed() const {
			noop();
			return false;
		}
		bool get_isZombie() const {
			noop();
			return false;
		}
		int32_t get_lrsLives() const {
			noop();
			return 0;
		}
		int32_t get_roasts() const {
			noop();
			return 0;
		}
		int32_t get_laps() const {
			noop();
			return 0;
		}
		int32_t get_lapTimeCurrent() const {
			noop();
			return 0;
		}
		int32_t get_lapTimes(uint32_t index) const {
			noop();
			return 0;
		}
		int32_t get_lapTimeBest() const {
			noop();
			return 0;
		}
		bool get_isAdmin() const {
			noop();
			return false;
		}
		bool hasPrivilege(const String& privilege, uint32_t moduleID) const {
			noop();
			return false;
		}

		bool doesCollide(const jjOBJ* object, bool always) const {
			noop();
			return false;
		}
		int getObjectHitForce(const jjOBJ& target) const {
			noop();
			return 0;
		}
		bool objectHit(jjOBJ* target, int force, uint32_t playerHandling) {
			noop();
			return false;
		}
		bool isEnemy(const jjPLAYER* victim) const {
			noop();
			return false;
		}

		uint32_t charCurr = 0;
		uint16_t curAnim = 0;
		uint32_t curFrame = 0;
		uint8_t frameID = 0;

	private:
		int _refCount;
		LevelScriptLoader* _levelScriptLoader;
		Actors::Player* _player;
	};

	int32_t get_jjPlayerCount() {
		auto ctx = asGetActiveContext();
		auto owner = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(ScriptLoader::EngineToOwner));

		void* mem = asAllocMem(sizeof(jjPLAYER));
		return owner->GetPlayers().size();
	}
	int32_t get_jjLocalPlayerCount() {
		auto ctx = asGetActiveContext();
		auto owner = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(ScriptLoader::EngineToOwner));

		void* mem = asAllocMem(sizeof(jjPLAYER));
		return owner->GetPlayers().size();
	}

	jjPLAYER* get_jjP() {
		noop();

		auto ctx = asGetActiveContext();
		auto owner = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(ScriptLoader::EngineToOwner));

		void* mem = asAllocMem(sizeof(jjPLAYER));
		return new(mem) jjPLAYER(owner, 0);
	}
	jjPLAYER* get_jjPlayers(uint8_t index) {
		noop();

		auto ctx = asGetActiveContext();
		auto owner = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(ScriptLoader::EngineToOwner));

		void* mem = asAllocMem(sizeof(jjPLAYER));
		return new(mem) jjPLAYER(owner, index);
	}
	jjPLAYER* get_jjLocalPlayers(uint8_t index) {
		noop();

		auto ctx = asGetActiveContext();
		auto owner = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(ScriptLoader::EngineToOwner));

		void* mem = asAllocMem(sizeof(jjPLAYER));
		return new(mem) jjPLAYER(owner, index);
	}

	class jjWEAPON
	{

	};

	class jjCHARACTER
	{

	};

	class jjLAYER;

	class jjPIXELMAP
	{
	public:
		jjPIXELMAP() : _refCount(1) {
			noop();
		}
		~jjPIXELMAP() {
			noop();
		}

		static jjPIXELMAP* CreateFromTile() {
			noop();

			auto ctx = asGetActiveContext();
			auto owner = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(ScriptLoader::EngineToOwner));

			void* mem = asAllocMem(sizeof(jjPIXELMAP));
			return new(mem) jjPIXELMAP();
		}
		static jjPIXELMAP* CreateFromSize(uint32_t width, uint32_t height) {
			noop();

			auto ctx = asGetActiveContext();
			auto owner = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(ScriptLoader::EngineToOwner));

			void* mem = asAllocMem(sizeof(jjPIXELMAP));
			return new(mem) jjPIXELMAP();
		}
		static jjPIXELMAP* CreateFromFrame(const jjANIMFRAME* animFrame) {
			noop();

			auto ctx = asGetActiveContext();
			auto owner = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(ScriptLoader::EngineToOwner));

			void* mem = asAllocMem(sizeof(jjPIXELMAP));
			return new(mem) jjPIXELMAP();
		}
		static jjPIXELMAP* CreateFromLayer(uint32_t left, uint32_t top, uint32_t width, uint32_t height, uint32_t layer) {
			noop();

			auto ctx = asGetActiveContext();
			auto owner = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(ScriptLoader::EngineToOwner));

			void* mem = asAllocMem(sizeof(jjPIXELMAP));
			return new(mem) jjPIXELMAP();
		}
		static jjPIXELMAP* CreateFromLayerObject(uint32_t left, uint32_t top, uint32_t width, uint32_t height, const jjLAYER* layer) {
			noop();

			auto ctx = asGetActiveContext();
			auto owner = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(ScriptLoader::EngineToOwner));

			void* mem = asAllocMem(sizeof(jjPIXELMAP));
			return new(mem) jjPIXELMAP();
		}
		static jjPIXELMAP* CreateFromTexture(uint32_t animFrame) {
			noop();

			auto ctx = asGetActiveContext();
			auto owner = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(ScriptLoader::EngineToOwner));

			void* mem = asAllocMem(sizeof(jjPIXELMAP));
			return new(mem) jjPIXELMAP();
		}
		static jjPIXELMAP* CreateFromFilename(const String& filename, const jjPAL* palette, uint8_t threshold) {
			noop();

			auto ctx = asGetActiveContext();
			auto owner = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(ScriptLoader::EngineToOwner));

			void* mem = asAllocMem(sizeof(jjPIXELMAP));
			return new(mem) jjPIXELMAP();
		}

		void AddRef()
		{
			_refCount++;
		}

		void Release()
		{
			if (--_refCount == 0) {
				this->~jjPIXELMAP();
				asFreeMem(this);
			}
		}

		// Assignment operator
		jjPIXELMAP& operator=(const jjPIXELMAP& o)
		{
			// Copy only the content, not the script proxy class
			//_value = o._value;
			return *this;
		}

		// TODO: return type uint8_t& instead?
		uint8_t GetPixel(uint32_t x, uint32_t y) {
			noop();
			return 0;
		}

		uint32_t width = 0;
		uint32_t height = 0;

		bool saveToTile(uint16_t tileID, bool hFlip) const {
			noop();
			return false;
		}
		bool saveToFrame(jjANIMFRAME* frame) const {
			noop();
			return false;
		}
		bool saveToFile(const String& filename, const jjPAL& palette) const {
			noop();
			return false;
		}

	private:
		int _refCount;
	};

	class jjMASKMAP
	{
	public:
		jjMASKMAP() : _refCount(1) {
			noop();
		}
		~jjMASKMAP() {
			noop();
		}

		static jjMASKMAP* CreateFromBool(bool filled) {
			noop();

			auto ctx = asGetActiveContext();
			auto owner = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(ScriptLoader::EngineToOwner));

			void* mem = asAllocMem(sizeof(jjMASKMAP));
			return new(mem) jjMASKMAP();
		}
		static jjMASKMAP* CreateFromTile(uint16_t tileID) {
			noop();

			auto ctx = asGetActiveContext();
			auto owner = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(ScriptLoader::EngineToOwner));

			void* mem = asAllocMem(sizeof(jjMASKMAP));
			return new(mem) jjMASKMAP();
		}

		void AddRef()
		{
			_refCount++;
		}

		void Release()
		{
			if (--_refCount == 0) {
				this->~jjMASKMAP();
				asFreeMem(this);
			}
		}

		// Assignment operator
		jjMASKMAP& operator=(const jjMASKMAP& o)
		{
			// Copy only the content, not the script proxy class
			//_value = o._value;
			return *this;
		}

		// TODO: return type bool& instead?
		bool GetPixel(uint32_t x, uint32_t y) {
			noop();
			return false;
		}

		bool save(uint16_t tileID, bool hFlip) const {
			noop();
			return false;
		}

	private:
		int _refCount;
	};

	class jjLAYER
	{
	public:
		jjLAYER() : _refCount(1) {
			noop();
		}
		~jjLAYER() {
			noop();
		}

		static jjLAYER* CreateFromSize(uint32_t width, uint32_t height, jjLAYER* self) {
			noop();
			return new(self) jjLAYER();
		}
		static jjLAYER* CreateCopy(jjLAYER* other, jjLAYER* self) {
			noop();
			return new(self) jjLAYER();
		}

		void AddRef()
		{
			_refCount++;
		}

		void Release()
		{
			if (--_refCount == 0) {
				this->~jjLAYER();
				asFreeMem(this);
			}
		}

		// Assignment operator
		jjLAYER& operator=(const jjLAYER& o)
		{
			// Copy only the content, not the script proxy class
			//_value = o._value;
			return *this;
		}

		static jjLAYER* get_jjLayers(int32_t index) {
			noop();

			auto ctx = asGetActiveContext();
			auto owner = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(ScriptLoader::EngineToOwner));

			void* mem = asAllocMem(sizeof(jjLAYER));
			return new(mem) jjLAYER();
		}

		int32_t width = 0;
		int32_t widthReal = 0;
		int32_t widthRounded = 0;
		int32_t height = 0;
		float xSpeed = 0;
		float ySpeed = 0;
		float xAutoSpeed = 0;
		float yAutoSpeed = 0;
		float xOffset = 0;
		float yOffset = 0;
		float xInnerSpeed = 0;
		float yInnerSpeed = 0;
		float xInnerAutoSpeed = 0;
		float yInnerAutoSpeed = 0;

		uint32_t get_spriteMode() const {
			noop();
			return 0;
		}
		uint32_t set_spriteMode(uint32_t value) const {
			noop();
			return 0;
		}
		uint8_t get_spriteParam() const {
			noop();
			return 0;
		}
		uint8_t set_spriteParam(uint8_t value) const {
			noop();
			return 0;
		}

		void setXSpeed(float newspeed, bool newSpeedIsAnAutoSpeed) const {
			noop();
		}
		void setYSpeed(float newspeed, bool newSpeedIsAnAutoSpeed) const {
			noop();
		}
		float getXPosition(const jjPLAYER* play) const {
			noop();
			return 0;
		}
		float getYPosition(const jjPLAYER* play) const {
			noop();
			return 0;
		}

		int32_t rotationAngle = 0;
		int32_t rotationRadiusMultiplier = 0;
		bool tileHeight = false;
		bool tileWidth = false;
		bool limitVisibleRegion = false;
		bool hasTileMap = false;
		bool hasTiles = false;

		static CScriptArray* jjLayerOrderGet() {
			noop();
			auto ctx = asGetActiveContext();

			auto engine = ctx->GetEngine();
			return CScriptArray::Create(engine->GetTypeInfoByDecl("array<jjLAYER@>"), 16);
		}
		static bool jjLayerOrderSet(const CScriptArray& order) {
			noop();
			return false;
		}
		static CScriptArray* jjLayersFromLevel(const String& filename, const CScriptArray& layerIDs, int32_t tileIDAdjustmentFactor) {
			noop();
			auto ctx = asGetActiveContext();
			auto engine = ctx->GetEngine();
			return CScriptArray::Create(engine->GetTypeInfoByDecl("array<jjLAYER@>"), 16);
		}
		static bool jjTilesFromTileset(const String& filename, uint32_t firstTileID, uint32_t tileCount, const CScriptArray* paletteColorMapping) {
			noop();
			return false;
		}

	private:
		int _refCount;
	};

	enum waterInteraction_ {
		waterInteraction_POSITIONBASED,
		waterInteraction_SWIM,
		waterInteraction_LOWGRAVITY
	};

	enum ws {
		wsNORMAL,
		wsMISSILE,
		wsPOPCORN,
		wsCAPPED,
	};

	enum wsp {
		wspNORMAL,
		wspNORMALORDIRECTIONANDAIM,
		wspDIRECTIONANDAIM,
		wspDOUBLEORTRIPLE,
		wspDOUBLE,
		wspTRIPLE,
		wspREFLECTSFASTFIRE,
		wspNORMALORBBGUN,
		wspBBGUN
	};

	constexpr int FLAG_HFLIPPED_TILE = 0x1000;
	constexpr int FLAG_VFLIPPED_TILE = 0x2000;
	constexpr int FLAG_ANIMATED_TILE = 0x4000;

	bool mlleSetup() {
		noop(); return true;
	}

	float get_sinTable(uint32_t angle) {
		noop();
		return sinf(angle * fTwoPi / 1024.0f);
	};
	float get_cosTable(uint32_t angle) {
		noop();
		return cosf(angle * fTwoPi / 1024.0f);
	};
	uint32_t RandWord32() {
		noop();
		return Random().Next();
	}
	uint64_t unixTimeSec() {
		noop(); return 0;
	}
	uint64_t unixTimeMs() {
		noop(); return 0;
	}

	uint32_t gameTicksSpentWhileActive = 0;
	int32_t renderFrame = 0;

	int32_t GetFPS() {
		noop(); return 0;
	}

	bool versionTSF = true;

	bool isAdmin() {
		noop(); return false;
	}

	bool isServer = false;

	bool jjDeactivatingBecauseOfDeath = false;

	int32_t GetDifficulty() {
		noop(); return 0;
	}
	int32_t SetDifficulty(int32_t value) {
		noop(); return 0;
	}

	int32_t DifficultyForNextLevel = 0;
	int32_t DifficultyAtLevelStart = 0;

	String getLevelFileName() {
		noop(); return "";
	}
	String getCurrLevelName() {
		noop(); return "";
	}
	void setCurrLevelName(const String& in) {
		noop();
	}
	String LevelScriptLoader::get_jjMusicFileName() {
		noop();

		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		return _this->_levelHandler->_musicCurrentPath;
	}
	String get_jjTilesetFileName() {
		noop(); return "";
	}

	uint32_t numberOfTiles = 0;

	String LevelScriptLoader::get_jjHelpStrings(uint32_t index) {
		noop();
		
		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		return _this->_levelHandler->GetLevelText(index);
	}
	void LevelScriptLoader::set_jjHelpStrings(uint32_t index, const String& text) {
		noop();

		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		_this->_levelHandler->OverrideLevelText(index, text);
	}

	int32_t get_gameState() {
		noop(); return 0;
	}

	int32_t gameMode = 0;
	int32_t customMode = 0;
	int32_t partyMode = 0;

	// TODO

	void LevelScriptLoader::jjAlert(const String& text, bool sendToAll, uint32_t size)
	{
		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		_this->_levelHandler->ShowLevelText(text);
	}
	void jjPrint(const String& text, bool timestamp) {
		LOGW("%s", text.data());
	}
	void jjDebug(const String& text, bool timestamp) {
		LOGD("%s", text.data());
	}
	void jjChat(const String& text, bool teamchat) {
		LOGW("%s", text.data());
	}
	void jjConsole(const String& text, bool sendToAll) {
		LOGW("%s", text.data());
	}
	void jjSpy(const String& text) {
		LOGD("%s", text.data());
	}

	// TODO

	bool parLowDetail = false;
	int32_t colorDepth = 0;
	int32_t checkedMaxSubVideoWidth = 0;
	int32_t checkedMaxSubVideoHeight = 0;
	int32_t realVideoW = 0;
	int32_t realVideoH = 0;
	int32_t subVideoW = 0;
	int32_t subVideoH = 0;
	int32_t getBorderWidth() {
		noop(); return 0;
	}
	int32_t getBorderHeight() {
		noop(); return 0;
	}
	bool getSplitscreenType() {
		noop(); return false;
	}
	bool setSplitscreenType() {
		noop(); return false;
	}

	// TODO

	int32_t maxScore = 0;

	int32_t get_teamScore(int32_t color) {
		noop(); return 0;
	}
	int32_t GetMaxHealth() {
		noop(); return 0;
	}
	int32_t GetStartHealth() {
		noop(); return 0;
	}

	// TODO

	float get_layerXOffset(uint8_t id) {
		noop(); return 0;
	}
	float set_layerXOffset(uint8_t id, float value) {
		noop(); return 0;
	}
	float get_layerYOffset(uint8_t id) {
		noop(); return 0;
	}
	float set_layerYOffset(uint8_t id, float value) {
		noop(); return 0;
	}
	int get_layerWidth(uint8_t id) {
		noop(); return 0;
	}
	int get_layerRealWidth(uint8_t id) {
		noop(); return 0;
	}
	int get_layerRoundedWidth(uint8_t id) {
		noop(); return 0;
	}
	int get_layerHeight(uint8_t id) {
		noop(); return 0;
	}
	float get_layerXSpeed(uint8_t id) {
		noop(); return 0;
	}
	float set_layerXSpeed(uint8_t id, float value) {
		noop(); return 0;
	}
	float get_layerYSpeed(uint8_t id) {
		noop(); return 0;
	}
	float set_layerYSpeed(uint8_t id, float value) {
		noop(); return 0;
	}
	float get_layerXAutoSpeed(uint8_t id) {
		noop(); return 0;
	}
	float set_layerXAutoSpeed(uint8_t id, float value) {
		noop(); return 0;
	}
	float get_layerYAutoSpeed(uint8_t id) {
		noop(); return 0;
	}
	float set_layerYAutoSpeed(uint8_t id, float value) {
		noop(); return 0;
	}
	bool get_layerHasTiles(uint8_t id) {
		noop(); return false;
	}
	bool set_layerHasTiles(uint8_t id, bool value) {
		noop(); return false;
	}
	bool get_layerTileHeight(uint8_t id) {
		noop(); return false;
	}
	bool set_layerTileHeight(uint8_t id, bool value) {
		noop(); return false;
	}
	bool get_layerTileWidth(uint8_t id) {
		noop(); return false;
	}
	bool set_layerTileWidth(uint8_t id, bool value) {
		noop(); return false;
	}
	bool get_layerLimitVisibleRegion(uint8_t id) {
		noop(); return false;
	}
	bool set_layerLimitVisibleRegion(uint8_t id, bool value) {
		noop(); return false;
	}

	void setLayerXSpeedSeamlessly(uint8_t id, float newspeed, bool newSpeedIsAnAutoSpeed) {
		noop();
	}
	void setLayerYSpeedSeamlessly(uint8_t id, float newspeed, bool newSpeedIsAnAutoSpeed) {
		noop();
	}

	// TODO

	void jjDrawPixel(float xPixel, float yPixel, uint8_t color, spriteType mode, uint8_t param, int8_t layerZ, uint8_t layerXY, int8_t playerID) {
		noop();
	}

	void jjDrawRectangle(float xPixel, float yPixel, int32_t width, int32_t height, uint8_t color, spriteType mode, uint8_t param, int8_t layerZ, uint8_t layerXY, int8_t playerID) {
		noop();
	}

	void jjDrawSprite(float xPixel, float yPixel, int32_t setID, uint8_t animation, uint8_t frame, int8_t direction, spriteType mode, uint8_t param, int8_t layerZ, uint8_t layerXY, int8_t playerID) {
		noop();
	}

	void jjDrawSpriteFromCurFrame(float xPixel, float yPixel, uint32_t sprite, int8_t direction, spriteType mode, uint8_t param, int8_t layerZ, uint8_t layerXY, int8_t playerID) {
		noop();
	}

	void jjDrawResizedSprite(float xPixel, float yPixel, int32_t setID, uint8_t animation, uint8_t frame, float xScale, float yScale, spriteType mode, uint8_t param, int8_t layerZ, uint8_t layerXY, int8_t playerID) {
		noop();
	}

	void jjDrawResizedSpriteFromCurFrame(float xPixel, float yPixel, uint32_t sprite, float xScale, float yScale, spriteType mode, uint8_t param, int8_t layerZ, uint8_t layerXY, int8_t playerID) {
		noop();
	}

	void jjDrawRotatedSprite(float xPixel, float yPixel, int32_t setID, uint8_t animation, uint8_t frame, int32_t angle, float xScale, float yScale, spriteType mode, uint8_t param, int8_t layerZ, uint8_t layerXY, int8_t playerID) {
		noop();
	}

	void jjDrawRotatedSpriteFromCurFrame(float xPixel, float yPixel, uint32_t sprite, int32_t angle, float xScale, float yScale, spriteType mode, uint8_t param, int8_t layerZ, uint8_t layerXY, int8_t playerID) {
		noop();
	}

	void jjDrawSwingingVineSpriteFromCurFrame(float xPixel, float yPixel, uint32_t sprite, int32_t length, int32_t curvature, spriteType mode, uint8_t param, int8_t layerZ, uint8_t layerXY, int8_t playerID) {
		noop();
	}

	void jjDrawTile(float xPixel, float yPixel, uint16_t tile, uint32_t tileQuadrant, int8_t layerZ, uint8_t layerXY, int8_t playerID) {
		noop();
	}

	void jjDrawString(float xPixel, float yPixel, const String& text, uint32_t size, uint32_t mode, uint8_t param, int8_t layerZ, uint8_t layerXY, int8_t playerID) {
		noop();
	}

	void jjDrawStringEx(float xPixel, float yPixel, const String& text, uint32_t size, const jjTEXTAPPEARANCE& appearance, uint8_t param1, spriteType spriteMode, uint8_t param2, int8_t layerZ, uint8_t layerXY, int8_t playerID) {
		noop();
	}

	int32_t jjGetStringWidth(const String& text, uint32_t size, const jjTEXTAPPEARANCE& style) {
		noop(); return 0;
	}

	bool snowing = false;
	bool snowingOutdoors = false;
	uint8_t snowingIntensity = 0;
	int32_t snowingType = 0;

	bool LevelScriptLoader::get_jjTriggers(uint8_t id) {
		//noop();
		
		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		auto tileMap = _this->_levelHandler->TileMap();
		return tileMap->GetTrigger(id);
	}
	bool LevelScriptLoader::set_jjTriggers(uint8_t id, bool value) {
		//noop();

		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		auto tileMap = _this->_levelHandler->TileMap();
		tileMap->SetTrigger(id, value);
		return value;
	}
	bool LevelScriptLoader::jjSwitchTrigger(uint8_t id) {
		//noop();

		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		auto tileMap = _this->_levelHandler->TileMap();
		tileMap->SetTrigger(id, !tileMap->GetTrigger(id));
		return tileMap->GetTrigger(id);
	}

	bool isNumberedASFunctionEnabled(uint8_t id) {
		noop(); return false;
	}
	bool setNumberedASFunctionEnabled(uint8_t id, bool value) {
		noop(); return false;
	}
	void reenableAllNumberedASFunctions() {
		noop();
	}

	int32_t waterLightMode = 0;
	int32_t waterInteraction = 0;

	float getWaterLevel() {
		noop(); return 0;
	}
	float getWaterLevel2() {
		noop(); return 0;
	}
	float setWaterLevel(float value, bool instant) {
		noop(); return 0;
	}
	float get_waterChangeSpeed() {
		noop(); return 0;
	}
	float set_waterChangeSpeed(float value) {
		noop(); return 0;
	}
	int32_t get_waterLayer() {
		noop(); return 0;
	}
	int32_t set_waterLayer(int32_t value) {
		noop(); return 0;
	}
	void setWaterGradient(uint8_t red1, uint8_t green1, uint8_t blue1, uint8_t red2, uint8_t green2, uint8_t blue2) {
		noop();
	}
	// TODO: void setWaterGradientFromColors(jjPALCOLOR color1, jjPALCOLOR color2)
	void setWaterGradientToTBG() {
		noop();
	}
	void resetWaterGradient() {
		noop();
	}

	void triggerRock(uint8_t id) {
		noop();
	}

	void cycleTo(const String& filename, bool warp, bool fast) {
		noop();
	}
	void LevelScriptLoader::jjNxt(bool warp, bool fast)
	{
		ExitType exitType = (warp ? ExitType::Warp : ExitType::Normal);
		if (fast) {
			exitType |= ExitType::FastTransition;
		}

		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		_this->_levelHandler->BeginLevelChange(exitType, { });
	}

	bool getEnabledTeam(uint8_t team) {
		noop(); return false;
	}

	uint8_t ChatKey = 0;

	bool getKeyDown(uint8_t key) {
		noop(); return false;
	}
	int32_t getCursorX() {
		noop(); return 0;
	}
	int32_t getCursorY() {
		noop(); return 0;
	}

	bool LevelScriptLoader::jjMusicLoad(const String& filename, bool forceReload, bool temporary) {
		noop();
		
		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		_this->_levelHandler->BeginPlayMusic(filename, !temporary, forceReload);
		
		return false;
	}
	void LevelScriptLoader::jjMusicStop() {
		noop();

		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		if (_this->_levelHandler->_music != nullptr) {
			_this->_levelHandler->_music->stop();
		}
	}
	void LevelScriptLoader::jjMusicPlay() {
		noop();

		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		if (_this->_levelHandler->_music != nullptr) {
			_this->_levelHandler->_music->play();
		}
	}
	void LevelScriptLoader::jjMusicPause() {
		noop();

		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		if (_this->_levelHandler->_music != nullptr) {
			_this->_levelHandler->_music->stop();
		}
	}
	void LevelScriptLoader::jjMusicResume() {
		noop();

		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		if (_this->_levelHandler->_music != nullptr && _this->_levelHandler->_music->isPaused()) {
			_this->_levelHandler->_music->play();
		}
	}

	void playSample(float xPixel, float yPixel, int32_t sample, int32_t volume, int32_t frequency) {
		noop();
	}
	int32_t playLoopedSample(float xPixel, float yPixel, int32_t sample, int32_t volume, int32_t frequency) {
		noop(); return 0;
	}
	void playPrioritySample(int32_t sample) {
		noop();
	}
	bool isSampleLoaded(int32_t sample) {
		noop(); return false;
	}
	bool loadSample(int32_t sample, const String& filename) {
		noop(); return false;
	}

	bool soundEnabled = false;
	bool soundFXActive = false;
	bool musicActive = false;
	int32_t soundFXVolume = false;
	int32_t musicVolume = false;
	int32_t levelEcho = 0;

	bool warpsTransmuteCoins = false;
	bool delayGeneratedCrateOrigins = false;

	bool getUseLayer8Speeds() {
		noop(); return false;
	}
	bool setUseLayer8Speeds(bool value) {
		noop(); return false;
	}

	bool g_levelHasFood = false;

	// TODO

	int32_t GetEvent(uint16_t tx, uint16_t ty) {
		noop(); return 0;
	}
	int32_t GetEventParamWrapper(uint16_t tx, uint16_t ty, int32_t offset, int32_t length) {
		noop(); return 0;
	}
	void SetEventByte(uint16_t tx, uint16_t ty, uint8_t newEventId) {
		noop();
	}
	void SetEventParam(uint16_t tx, uint16_t ty, int8_t offset, int8_t length, int32_t newValue) {
		noop();
	}
	int8_t GetTileType(uint16_t tile) {
		noop(); return 0;
	}
	int8_t SetTileType(uint16_t tile, uint16_t value) {
		noop(); return 0;
	}

	int32_t enforceAmbientLighting = 0;

	// TODO

	uint16_t jjGetStaticTile(uint16_t tileID) {
		noop();
		return 0;
	}
	uint16_t jjTileGet(uint8_t layer, int32_t xTile, int32_t yTile) {
		noop();
		return 0;
	}
	uint16_t jjTileSet(uint8_t layer, int32_t xTile, int32_t yTile, uint16_t newTile) {
		noop();
		return 0;
	}
	void jjGenerateSettableTileArea(uint8_t layer, int32_t xTile, int32_t yTile, int32_t width, int32_t height) {
		noop();
	}

	// TODO
	
	bool jjMaskedPixel(int32_t xPixel, int32_t yPixel) {
		noop();
		return false;
	}
	bool jjMaskedPixelLayer(int32_t xPixel, int32_t yPixel, uint8_t layer) {
		noop();
		return false;
	}
	bool jjMaskedHLine(int32_t xPixel, int32_t lineLength, int32_t yPixel) {
		noop();
		return false;
	}
	bool jjMaskedHLineLayer(int32_t xPixel, int32_t lineLength, int32_t yPixel, uint8_t layer) {
		noop();
		return false;
	}
	bool jjMaskedVLine(int32_t xPixel, int32_t yPixel, int32_t lineLength) {
		noop();
		return false;
	}
	bool jjMaskedVLineLayer(int32_t xPixel, int32_t yPixel, int32_t lineLength, uint8_t layer) {
		noop();
		return false;
	}
	bool jjMaskedTopVLine(int32_t xPixel, int32_t yPixel, int32_t lineLength) {
		noop();
		return false;
	}
	bool jjMaskedTopVLineLayer(int32_t xPixel, int32_t yPixel, int32_t lineLength, uint8_t layer) {
		noop();
		return false;
	}

	// TODO

	void jjSetModPosition(int32_t order, int32_t row, bool reset) {
		noop();
	}
	void jjSlideModChannelVolume(int32_t channel, float volume, int32_t milliseconds) {
		noop();
	}
	int32_t jjGetModOrder() {
		noop();
		return 0;
	}
	int32_t jjGetModRow() {
		noop();
		return 0;
	}
	int32_t jjGetModTempo() {
		noop();
		return 0;
	}
	void jjSetModTempo(uint8_t speed) {
		noop();
	}
	int32_t jjGetModSpeed() {
		noop();
		return 0;
	}
	void jjSetModSpeed(uint8_t speed) {
		noop();
	}

	uint32_t getCustomSetID(uint8_t index) {
		noop();
		return mCOUNT + index;
	}

	// Without namespace for shorter log messages
	static void asScript(String& msg)
	{
		LOGI("%s", msg.data());
	}

	static float asFractionf(float v)
	{
		float intPart;
		return modff(v, &intPart);
	}

	static int asRandom()
	{
		return Random().Next();
	}

	static int asRandom(int max)
	{
		return Random().Fast(0, max);
	}

	static float asRandom(float min, float max)
	{
		return Random().FastFloat(min, max);
	}

	LevelScriptLoader::LevelScriptLoader(LevelHandler* levelHandler, const StringView& scriptPath)
		:
		_levelHandler(levelHandler),
		_onLevelUpdate(nullptr),
		_onLevelUpdateLastFrame(-1)
	{
		// Try to load the script
		HashMap<String, bool> DefinedSymbols = {
#if defined(DEATH_TARGET_EMSCRIPTEN)
			{ "TARGET_EMSCRIPTEN"_s, true },
#elif defined(DEATH_TARGET_ANDROID)
			{ "TARGET_ANDROID"_s, true },
#elif defined(DEATH_TARGET_APPLE)
			{ "TARGET_APPLE"_s, true },
#	if defined(DEATH_TARGET_IOS)
			{ "TARGET_IOS"_s, true },
#	endif
#elif defined(DEATH_TARGET_WINDOWS)
			{ "TARGET_WINDOWS"_s, true },
#	if defined(DEATH_TARGET_WINDOWS_RT)
			{ "TARGET_WINDOWS_RT"_s, true },
#	endif
#elif defined(DEATH_TARGET_UNIX)
			{ "TARGET_UNIX"_s, true },
#endif

#if defined(DEATH_TARGET_BIG_ENDIAN)
			{ "TARGET_BIG_ENDIAN"_s, true },
#endif

#if defined(WITH_OPENGLES)
			{ "WITH_OPENGLES"_s, true },
#endif
#if defined(WITH_AUDIO)
			{ "WITH_AUDIO"_s, true },
#endif
#if defined(WITH_VORBIS)
			{ "WITH_VORBIS"_s, true },
#endif
#if defined(WITH_OPENMPT)
			{ "WITH_OPENMPT"_s, true },
#endif
#if defined(WITH_THREADS)
			{ "WITH_THREADS"_s, true },
#endif
			{ "Resurrection"_s, true }
		};

		_scriptContextType = AddScriptFromFile(scriptPath, DefinedSymbols);
		if (_scriptContextType == ScriptContextType::Unknown) {
			LOGE("Cannot compile the script. Please correct the code and try again.");
			return;
		}

		RegisterBuiltInFunctions(_engine);
		switch (_scriptContextType) {
			case ScriptContextType::Legacy:
				LOGD("Compiling script with \"Legacy\" context");
				RegisterLegacyFunctions(_engine);
				break;
			case ScriptContextType::Standard:
				LOGD("Compiling script with \"Standard\" context");
				RegisterStandardFunctions(_engine, _module);
				break;
		}

		int r = Build(); RETURN_ASSERT_MSG(r >= 0, "Cannot compile the script. Please correct the code and try again.");

		switch (_scriptContextType) {
			case ScriptContextType::Legacy:
				_onLevelUpdate = _module->GetFunctionByDecl("void onMain()");
				break;
			case ScriptContextType::Standard:
				_onLevelUpdate = _module->GetFunctionByDecl("void onLevelUpdate(float)");
				break;
		}
	}

	String LevelScriptLoader::OnProcessInclude(const StringView& includePath, const StringView& scriptPath)
	{
		// Skip MLLE files, because it's handled natively
		if (includePath.hasPrefix("MLLE-Include-"_s) && includePath.hasSuffix(".asc"_s)) {
			return { };
		}

		// TODO: Allow multiple search paths
		//return ConstructPath(includePath, path);

		auto sourcePath = ContentResolver::Get().GetSourcePath();
		return fs::CombinePath(sourcePath, includePath);
	}

	void LevelScriptLoader::OnProcessPragma(const StringView& content, ScriptContextType& contextType)
	{
		// #pragma target Jazz² Resurrection - Changes script context type to Standard
		if (content == "target Jazz² Resurrection"_s || content == "target Jazz2 Resurrection"_s) {
			contextType = ScriptContextType::Standard;
		}
	}

	void LevelScriptLoader::OnLevelLoad()
	{
		asIScriptFunction* func = _module->GetFunctionByDecl("void onLevelLoad()");
		if (func == nullptr) {
			return;
		}

		asIScriptContext* ctx = _engine->RequestContext();

		ctx->Prepare(func);
		int r = ctx->Execute();
		if (r == asEXECUTION_EXCEPTION) {
			OnException(ctx);
		}

		_engine->ReturnContext(ctx);
	}

	void LevelScriptLoader::OnLevelBegin()
	{
		asIScriptFunction* func = _module->GetFunctionByDecl("void onLevelBegin()");
		if (func == nullptr) {
			return;
		}

		asIScriptContext* ctx = _engine->RequestContext();

		ctx->Prepare(func);
		int r = ctx->Execute();
		if (r == asEXECUTION_EXCEPTION) {
			OnException(ctx);
		}

		_engine->ReturnContext(ctx);
	}

	void LevelScriptLoader::OnLevelReload()
	{
		asIScriptFunction* func = _module->GetFunctionByDecl("void onLevelReload()");
		if (func == nullptr) {
			return;
		}

		asIScriptContext* ctx = _engine->RequestContext();

		ctx->Prepare(func);
		int r = ctx->Execute();
		if (r == asEXECUTION_EXCEPTION) {
			OnException(ctx);
		}

		_engine->ReturnContext(ctx);
	}

	void LevelScriptLoader::OnLevelUpdate(float timeMult)
	{
		switch (_scriptContextType) {
			case ScriptContextType::Legacy: {
				asIScriptFunction* onPlayer = _module->GetFunctionByName("void onPlayer(jjPLAYER@)");

				if (_onLevelUpdate == nullptr && onPlayer == nullptr) {
					_onLevelUpdateLastFrame = (int32_t)_levelHandler->_elapsedFrames;
					return;
				}

				// Legacy context requires fixed frame count per second
				asIScriptContext* ctx = _engine->RequestContext();

				// It should update at 70 FPS instead of 60 FPS
				int32_t currentFrame = (int32_t)(_levelHandler->_elapsedFrames * (70.0f / 60.0f));
				while (_onLevelUpdateLastFrame <= currentFrame) {
					if (_onLevelUpdate != nullptr) {
						ctx->Prepare(_onLevelUpdate);
						int r = ctx->Execute();
						if (r == asEXECUTION_EXCEPTION) {
							OnException(ctx);
							// Don't call the method again if an exception occurs
							_onLevelUpdate = nullptr;
						}
					}
					if (onPlayer != nullptr) {
						for (auto player : _levelHandler->_players) {
							ctx->Prepare(onPlayer);

							void* mem = asAllocMem(sizeof(jjPLAYER));
							jjPLAYER* playerWrapper = new(mem) jjPLAYER(this, player);
							ctx->SetArgObject(0, playerWrapper);

							int r = ctx->Execute();
							if (r == asEXECUTION_EXCEPTION) {
								OnException(ctx);
								// Don't call the method again if an exception occurs
								//_onLevelUpdate = nullptr;
							}

							playerWrapper->Release();
						}
					}
					_onLevelUpdateLastFrame++;
				}

				_engine->ReturnContext(ctx);
				break;
			}
			case ScriptContextType::Standard: {
				_onLevelUpdateLastFrame = (int32_t)_levelHandler->_elapsedFrames;
				if (_onLevelUpdate == nullptr) {
					return;
				}

				// Standard context supports floating frame rate
				asIScriptContext* ctx = _engine->RequestContext();
				ctx->Prepare(_onLevelUpdate);
				ctx->SetArgFloat(0, timeMult);
				int r = ctx->Execute();
				if (r == asEXECUTION_EXCEPTION) {
					LOGE("An exception \"%s\" occurred in \"%s\". Please correct the code and try again.", ctx->GetExceptionString(), ctx->GetExceptionFunction()->GetDeclaration());
					// Don't call the method again if an exception occurs
					_onLevelUpdate = nullptr;
				}

				_engine->ReturnContext(ctx);
				break;
			}
		}
	}

	void LevelScriptLoader::OnLevelCallback(Actors::ActorBase* initiator, uint8_t* eventParams)
	{
		char funcName[32];
		formatString(funcName, sizeof(funcName), "onFunction%i", eventParams[0]);
		asIScriptFunction* func = _module->GetFunctionByName(funcName);
		if (func != nullptr) {
			asIScriptContext* ctx = _engine->RequestContext();
			ctx->Prepare(func);

			jjPLAYER* playerWrapper = nullptr;
			int paramIdx = 0;
			int typeId = 0;
			if (func->GetParam(paramIdx, &typeId) >= 0) {
				if ((typeId & (asTYPEID_OBJHANDLE | asTYPEID_APPOBJECT)) == (asTYPEID_OBJHANDLE | asTYPEID_APPOBJECT)) {
					asITypeInfo* typeInfo = _engine->GetTypeInfoById(typeId);
					if (typeInfo->GetName() == "jjPLAYER"_s) {
						void* mem = asAllocMem(sizeof(jjPLAYER));
						playerWrapper = new(mem) jjPLAYER(this, _levelHandler->_players[0]);
						ctx->SetArgObject(0, playerWrapper);
					}
					paramIdx++;
				}
			}
			if (func->GetParam(paramIdx, &typeId) >= 0) {
				if (typeId == asTYPEID_BOOL || typeId == asTYPEID_INT8 || typeId == asTYPEID_UINT8) {
					ctx->SetArgByte(1, eventParams[1]);
					paramIdx++;
				}
			}

			int r = ctx->Execute();
			if (r == asEXECUTION_EXCEPTION) {
				LOGE("An exception \"%s\" occurred in \"%s\". Please correct the code and try again.", ctx->GetExceptionString(), ctx->GetExceptionFunction()->GetDeclaration());
			}

			_engine->ReturnContext(ctx);

			if (playerWrapper != nullptr) {
				playerWrapper->Release();
			}
			return;
		}

		/*
		// If known player is the initiator, try to call specific variant of the function
		if (auto player = dynamic_cast<Actors::Player*>(initiator)) {
			formatString(funcName, sizeof(funcName), "void onFunction%i(Player@, uint8)", eventParams[0]);
			func = _module->GetFunctionByDecl(funcName);
			if (func != nullptr) {
				asIScriptContext* ctx = _engine->RequestContext();

				void* mem = asAllocMem(sizeof(jjPLAYER));
				jjPLAYER* playerWrapper = new(mem) jjPLAYER(this, player);

				ctx->Prepare(func);
				ctx->SetArgObject(0, playerWrapper);
				ctx->SetArgByte(1, eventParams[1]);
				int r = ctx->Execute();
				if (r == asEXECUTION_EXCEPTION) {
					LOGE("An exception \"%s\" occurred in \"%s\". Please correct the code and try again.", ctx->GetExceptionString(), ctx->GetExceptionFunction()->GetDeclaration());
				}

				_engine->ReturnContext(ctx);

				playerWrapper->Release();
				return;
			}
		}

		// Try to call parameter-less variant
		formatString(funcName, sizeof(funcName), "void onFunction%i()", eventParams[0]);
		func = _module->GetFunctionByDecl(funcName);
		if (func != nullptr) {
			asIScriptContext* ctx = _engine->RequestContext();

			ctx->Prepare(func);
			int r = ctx->Execute();
			if (r == asEXECUTION_EXCEPTION) {
				LOGE("An exception \"%s\" occurred in \"%s\". Please correct the code and try again.", ctx->GetExceptionString(), ctx->GetExceptionFunction()->GetDeclaration());
			}

			_engine->ReturnContext(ctx);
			return;
		}*/

		LOGW("Callback function \"%s\" was not found in the script. Please correct the code and try again.", funcName);
	}

	void LevelScriptLoader::RegisterBuiltInFunctions(asIScriptEngine* engine)
	{
		RegisterRef(engine);
		RegisterString(engine);
		RegisterArray(engine);
		RegisterDictionary(engine);

		// Math functions
		int r;
		r = engine->RegisterGlobalFunction("float cos(float)", asFUNCTIONPR(cosf, (float), float), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("float sin(float)", asFUNCTIONPR(sinf, (float), float), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("float tan(float)", asFUNCTIONPR(tanf, (float), float), asCALL_CDECL); RETURN_ASSERT(r >= 0);

		r = engine->RegisterGlobalFunction("float acos(float)", asFUNCTIONPR(acosf, (float), float), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("float asin(float)", asFUNCTIONPR(asinf, (float), float), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("float atan(float)", asFUNCTIONPR(atanf, (float), float), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("float atan2(float, float)", asFUNCTIONPR(atan2f, (float, float), float), asCALL_CDECL); RETURN_ASSERT(r >= 0);

		r = engine->RegisterGlobalFunction("float cosh(float)", asFUNCTIONPR(coshf, (float), float), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("float sinh(float)", asFUNCTIONPR(sinhf, (float), float), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("float tanh(float)", asFUNCTIONPR(tanhf, (float), float), asCALL_CDECL); RETURN_ASSERT(r >= 0);

		r = engine->RegisterGlobalFunction("float log(float)", asFUNCTIONPR(logf, (float), float), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("float log10(float)", asFUNCTIONPR(log10f, (float), float), asCALL_CDECL); RETURN_ASSERT(r >= 0);

		r = engine->RegisterGlobalFunction("float pow(float, float)", asFUNCTIONPR(powf, (float, float), float), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("float sqrt(float)", asFUNCTIONPR(sqrtf, (float), float), asCALL_CDECL); RETURN_ASSERT(r >= 0);

		r = engine->RegisterGlobalFunction("float ceil(float)", asFUNCTIONPR(ceilf, (float), float), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("float abs(float)", asFUNCTIONPR(fabsf, (float), float), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("float floor(float)", asFUNCTIONPR(floorf, (float), float), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("float fraction(float)", asFUNCTIONPR(asFractionf, (float), float), asCALL_CDECL); RETURN_ASSERT(r >= 0);
	}

	void LevelScriptLoader::RegisterLegacyFunctions(asIScriptEngine* engine)
	{
		// JJ2+ Declarations (provided by JJ2+ team)
		engine->SetDefaultNamespace("");
		engine->RegisterGlobalFunction("float jjSin(uint angle)", asFUNCTION(get_sinTable), asCALL_CDECL);
		engine->RegisterGlobalFunction("float jjCos(uint angle)", asFUNCTION(get_cosTable), asCALL_CDECL);
		engine->RegisterGlobalFunction("uint jjRandom()", asFUNCTION(RandWord32), asCALL_CDECL);
		engine->RegisterGlobalFunction("uint64 jjUnixTimeSec()", asFUNCTION(unixTimeSec), asCALL_CDECL);
		engine->RegisterGlobalFunction("uint64 jjUnixTimeMs()", asFUNCTION(unixTimeMs), asCALL_CDECL);

		//engine->RegisterGlobalFunction("bool jjIsValidCheat(const string &in text)", asFUNCTION(stringIsCheat), asCALL_CDECL);
		//engine->RegisterGlobalProperty("const bool jjDebugF10", &F10Debug);

		//engine->RegisterGlobalFunction("bool jjRegexIsValid(const string &in expression)", asFUNCTION(regexIsValid), asCALL_CDECL);
		//engine->RegisterGlobalFunction("bool jjRegexMatch(const string &in text, const string &in expression, bool ignoreCase = false)", asFUNCTION(regexMatch), asCALL_CDECL);
		//engine->RegisterGlobalFunction("bool jjRegexMatch(const string &in text, const string &in expression, array<string> &out results, bool ignoreCase = false)", asFUNCTION(regexMatchWithResults), asCALL_CDECL);
		//engine->RegisterGlobalFunction("bool jjRegexSearch(const string &in text, const string &in expression, bool ignoreCase = false)", asFUNCTION(regexSearch), asCALL_CDECL);
		//engine->RegisterGlobalFunction("bool jjRegexSearch(const string &in text, const string &in expression, array<string> &out results, bool ignoreCase = false)", asFUNCTION(regexSearchWithResults), asCALL_CDECL);
		//engine->RegisterGlobalFunction("string jjRegexReplace(const string &in text, const string &in expression, const string &in replacement, bool ignoreCase= false)", asFUNCTION(regexReplace), asCALL_CDECL);

		engine->RegisterGlobalProperty("const int jjGameTicks", &_onLevelUpdateLastFrame);
		engine->RegisterGlobalProperty("const uint jjActiveGameTicks", &gameTicksSpentWhileActive);
		engine->RegisterGlobalProperty("const int jjRenderFrame", &renderFrame);
		engine->RegisterGlobalFunction("int get_jjFPS()", asFUNCTION(GetFPS), asCALL_CDECL);
		engine->RegisterGlobalProperty("const bool jjIsTSF", &versionTSF);
		engine->RegisterGlobalFunction("bool get_jjIsAdmin()", asFUNCTION(isAdmin), asCALL_CDECL);
		engine->RegisterGlobalProperty("const bool jjIsServer", &isServer);
		engine->RegisterGlobalFunction("int get_jjDifficulty()", asFUNCTION(GetDifficulty), asCALL_CDECL);
		engine->RegisterGlobalFunction("int set_jjDifficulty(int)", asFUNCTION(SetDifficulty), asCALL_CDECL);
		engine->RegisterGlobalProperty("int jjDifficultyNext", &DifficultyForNextLevel);
		engine->RegisterGlobalProperty("const int jjDifficultyOrig", &DifficultyAtLevelStart);

		engine->RegisterGlobalFunction("string get_jjLevelFileName()", asFUNCTION(getLevelFileName), asCALL_CDECL);
		engine->RegisterGlobalFunction("string get_jjLevelName()", asFUNCTION(getCurrLevelName), asCALL_CDECL);
		engine->RegisterGlobalFunction("void set_jjLevelName(const string &in)", asFUNCTION(setCurrLevelName), asCALL_CDECL);
		engine->RegisterGlobalFunction("string get_jjMusicFileName()", asFUNCTION(get_jjMusicFileName), asCALL_CDECL);
		engine->RegisterGlobalFunction("string get_jjTilesetFileName()", asFUNCTION(get_jjTilesetFileName), asCALL_CDECL);
		engine->RegisterGlobalProperty("const uint jjTileCount", &numberOfTiles);

		engine->RegisterGlobalFunction("string get_jjHelpStrings(uint)", asFUNCTION(get_jjHelpStrings), asCALL_CDECL);
		engine->RegisterGlobalFunction("void set_jjHelpStrings(uint, const string &in)", asFUNCTION(set_jjHelpStrings), asCALL_CDECL);

		engine->SetDefaultNamespace("GAME");
		engine->RegisterEnum("State");
		engine->RegisterEnumValue("State", "STOPPED", gameSTOPPED);
		engine->RegisterEnumValue("State", "STARTED", gameSTARTED);
		engine->RegisterEnumValue("State", "PAUSED", gamePAUSED);
		engine->RegisterEnumValue("State", "PREGAME", gamePREGAME);
		engine->RegisterEnumValue("State", "OVERTIME", gameOVERTIME);
		engine->RegisterEnum("Mode");
		engine->RegisterEnumValue("Mode", "SP", GM_SP);
		engine->RegisterEnumValue("Mode", "COOP", GM_COOP);
		engine->RegisterEnumValue("Mode", "BATTLE", GM_BATTLE);
		engine->RegisterEnumValue("Mode", "CTF", GM_CTF);
		engine->RegisterEnumValue("Mode", "TREASURE", GM_TREASURE);
		engine->RegisterEnumValue("Mode", "RACE", GM_RACE);
		engine->RegisterEnum("Custom");
		engine->RegisterEnumValue("Custom", "NOCUSTOM", 0);
		engine->RegisterEnumValue("Custom", "RT", 1);
		engine->RegisterEnumValue("Custom", "LRS", 2);
		engine->RegisterEnumValue("Custom", "XLRS", 3);
		engine->RegisterEnumValue("Custom", "PEST", 4);
		engine->RegisterEnumValue("Custom", "TB", 5);
		engine->RegisterEnumValue("Custom", "JB", 6);
		engine->RegisterEnumValue("Custom", "DCTF", 7);
		engine->RegisterEnumValue("Custom", "FR", 8);
		engine->RegisterEnumValue("Custom", "TLRS", 9);
		engine->RegisterEnumValue("Custom", "DOM", 10);
		engine->RegisterEnumValue("Custom", "HEAD", 11);
		engine->RegisterEnum("Connection");
		engine->RegisterEnumValue("Connection", "LOCAL", gameLOCAL);
		engine->RegisterEnumValue("Connection", "ONLINE", gameINTERNET);
		engine->RegisterEnumValue("Connection", "LAN", gameLAN_TCP);
		engine->SetDefaultNamespace("");
		engine->RegisterGlobalFunction("GAME::State get_jjGameState()", asFUNCTION(get_gameState), asCALL_CDECL);
		engine->RegisterGlobalProperty("const GAME::Mode jjGameMode", &gameMode);
		engine->RegisterGlobalProperty("const GAME::Custom jjGameCustom", &customMode);
		engine->RegisterGlobalProperty("const GAME::Connection jjGameConnection", &partyMode);

		// TODO
		engine->RegisterObjectType("jjPLAYER", sizeof(jjPLAYER), asOBJ_REF /*| asOBJ_NOCOUNT*/);
		engine->RegisterObjectBehaviour("jjPLAYER", asBEHAVE_ADDREF, "void f()", asMETHOD(jjPLAYER, AddRef), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("jjPLAYER", asBEHAVE_RELEASE, "void f()", asMETHOD(jjPLAYER, Release), asCALL_THISCALL);
		engine->RegisterGlobalFunction("const int get_jjPlayerCount()", asFUNCTION(get_jjPlayerCount), asCALL_CDECL);
		engine->RegisterGlobalFunction("const int get_jjLocalPlayerCount()", asFUNCTION(get_jjLocalPlayerCount), asCALL_CDECL);
		engine->RegisterGlobalFunction("jjPLAYER@ get_jjP()", asFUNCTION(get_jjP), asCALL_CDECL); // Deprecated
		engine->RegisterGlobalFunction("jjPLAYER@ get_p()", asFUNCTION(get_jjP), asCALL_CDECL); // Deprecated
		engine->RegisterGlobalFunction("jjPLAYER@ get_jjPlayers(uint8)", asFUNCTION(get_jjPlayers), asCALL_CDECL);
		engine->RegisterGlobalFunction("jjPLAYER@ get_jjLocalPlayers(uint8)", asFUNCTION(get_jjLocalPlayers), asCALL_CDECL);

		engine->SetDefaultNamespace("WEAPON");
		engine->RegisterEnum("Weapon");
		engine->RegisterEnumValue("Weapon", "BLASTER", 1);
		engine->RegisterEnumValue("Weapon", "BOUNCER", 2);
		engine->RegisterEnumValue("Weapon", "ICE", 3);
		engine->RegisterEnumValue("Weapon", "SEEKER", 4);
		engine->RegisterEnumValue("Weapon", "RF", 5);
		engine->RegisterEnumValue("Weapon", "TOASTER", 6);
		engine->RegisterEnumValue("Weapon", "TNT", 7);
		engine->RegisterEnumValue("Weapon", "GUN8", 8);
		engine->RegisterEnumValue("Weapon", "GUN9", 9);
		engine->RegisterEnumValue("Weapon", "CURRENT", 0);
		engine->RegisterEnum("Style");
		engine->RegisterEnumValue("Style", "NORMAL", wsNORMAL);
		engine->RegisterEnumValue("Style", "MISSILE", wsMISSILE);
		engine->RegisterEnumValue("Style", "POPCORN", wsPOPCORN);
		engine->RegisterEnumValue("Style", "CAPPED", wsCAPPED);
		engine->RegisterEnumValue("Style", "TUNA", wsCAPPED); // Deprecated
		engine->SetDefaultNamespace("SPREAD");
		engine->RegisterEnum("Spread");
		engine->RegisterEnumValue("Spread", "NORMAL", wspNORMAL);
		engine->RegisterEnumValue("Spread", "ICE", wspNORMALORDIRECTIONANDAIM);
		engine->RegisterEnumValue("Spread", "ICEPU", wspDIRECTIONANDAIM);
		engine->RegisterEnumValue("Spread", "RF", wspDOUBLEORTRIPLE);
		engine->RegisterEnumValue("Spread", "RFNORMAL", wspDOUBLE);
		engine->RegisterEnumValue("Spread", "RFPU", wspTRIPLE);
		engine->RegisterEnumValue("Spread", "TOASTER", wspREFLECTSFASTFIRE);
		engine->RegisterEnumValue("Spread", "GUN8", wspNORMALORBBGUN);
		engine->RegisterEnumValue("Spread", "PEPPERSPRAY", wspBBGUN);
		engine->SetDefaultNamespace("GEM");
		engine->RegisterEnum("Color");
		engine->RegisterEnumValue("Color", "RED", 1);
		engine->RegisterEnumValue("Color", "GREEN", 2);
		engine->RegisterEnumValue("Color", "BLUE", 3);
		engine->RegisterEnumValue("Color", "PURPLE", 4);
		engine->SetDefaultNamespace("SHIELD");
		engine->RegisterEnum("Shield");
		engine->RegisterEnumValue("Shield", "NONE", 0);
		engine->RegisterEnumValue("Shield", "FIRE", 1);
		engine->RegisterEnumValue("Shield", "BUBBLE", 2);
		engine->RegisterEnumValue("Shield", "WATER", 2);
		engine->RegisterEnumValue("Shield", "LIGHTNING", 3);
		engine->RegisterEnumValue("Shield", "PLASMA", 3);
		engine->RegisterEnumValue("Shield", "LASER", 4);

		engine->SetDefaultNamespace("");
		engine->RegisterObjectProperty("jjPLAYER", "int score", asOFFSET(jjPLAYER, score));
		engine->RegisterObjectProperty("jjPLAYER", "int scoreDisplayed", asOFFSET(jjPLAYER, lastScoreDisplay));
		engine->RegisterObjectMethod("jjPLAYER", "int setScore(int score)", asMETHOD(jjPLAYER, setScore), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjPLAYER", "float xPos", asOFFSET(jjPLAYER, xPos));
		engine->RegisterObjectProperty("jjPLAYER", "float yPos", asOFFSET(jjPLAYER, yPos));
		engine->RegisterObjectProperty("jjPLAYER", "float xAcc", asOFFSET(jjPLAYER, xAcc));
		engine->RegisterObjectProperty("jjPLAYER", "float yAcc", asOFFSET(jjPLAYER, yAcc));
		engine->RegisterObjectProperty("jjPLAYER", "float xOrg", asOFFSET(jjPLAYER, xOrg));
		engine->RegisterObjectProperty("jjPLAYER", "float yOrg", asOFFSET(jjPLAYER, yOrg));
		engine->RegisterObjectMethod("jjPLAYER", "float get_xSpeed() const", asMETHOD(jjPLAYER, get_xSpeed), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "float set_xSpeed(float)", asMETHOD(jjPLAYER, set_xSpeed), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "float get_ySpeed() const", asMETHOD(jjPLAYER, get_ySpeed), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "float set_ySpeed(float)", asMETHOD(jjPLAYER, set_ySpeed), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjPLAYER", "float jumpStrength", asOFFSET(jjPLAYER, jumpStrength));
		engine->RegisterObjectProperty("jjPLAYER", "int8 frozen", asOFFSET(jjPLAYER, frozen));
		engine->RegisterObjectMethod("jjPLAYER", "void freeze(bool frozen = true)", asMETHOD(jjPLAYER, freeze), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "int get_currTile() const", asMETHOD(jjPLAYER, get_currTile), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool startSugarRush(int time = 1400)", asMETHOD(jjPLAYER, startSugarRush), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "int8 get_health() const", asMETHOD(jjPLAYER, get_health), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "int8 set_health(int8)", asMETHOD(jjPLAYER, set_health), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjPLAYER", "const int warpID", asOFFSET(jjPLAYER, warpID));
		engine->RegisterObjectProperty("jjPLAYER", "int fastfire", asOFFSET(jjPLAYER, fastfire));
		engine->RegisterObjectMethod("jjPLAYER", "uint8 get_currWeapon() const", asMETHOD(jjPLAYER, get_currWeapon), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "uint8 set_currWeapon(uint8)", asMETHOD(jjPLAYER, set_currWeapon), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjPLAYER", "int lives", asOFFSET(jjPLAYER, lives));
		engine->RegisterObjectProperty("jjPLAYER", "int invincibility", asOFFSET(jjPLAYER, invincibility));
		engine->RegisterObjectProperty("jjPLAYER", "int blink", asOFFSET(jjPLAYER, blink));
		engine->RegisterObjectMethod("jjPLAYER", "int extendInvincibility(int duration)", asMETHOD(jjPLAYER, extendInvincibility), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjPLAYER", "int food", asOFFSET(jjPLAYER, food));
		engine->RegisterObjectProperty("jjPLAYER", "int coins", asOFFSET(jjPLAYER, coins));
		engine->RegisterObjectMethod("jjPLAYER", "bool testForCoins(int numberOfCoins)", asMETHOD(jjPLAYER, testForCoins), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "int get_gems(GEM::Color) const", asMETHOD(jjPLAYER, get_gems), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "int set_gems(GEM::Color, int)", asMETHOD(jjPLAYER, set_gems), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool testForGems(int numberOfGems, GEM::Color type)", asMETHOD(jjPLAYER, testForGems), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjPLAYER", "int shieldType", asOFFSET(jjPLAYER, shieldType));
		engine->RegisterObjectProperty("jjPLAYER", "int shieldTime", asOFFSET(jjPLAYER, shieldTime));
		engine->RegisterObjectProperty("jjPLAYER", "int ballTime", asOFFSET(jjPLAYER, rolling));
		engine->RegisterObjectProperty("jjPLAYER", "int boss", asOFFSET(jjPLAYER, bossNumber));
		engine->RegisterObjectProperty("jjPLAYER", "bool bossActivated", asOFFSET(jjPLAYER, bossActive));
		engine->RegisterObjectProperty("jjPLAYER", "int8 direction", asOFFSET(jjPLAYER, direction));
		engine->RegisterObjectProperty("jjPLAYER", "int platform", asOFFSET(jjPLAYER, platform));
		engine->RegisterObjectProperty("jjPLAYER", "const int flag", asOFFSET(jjPLAYER, flag));
		engine->RegisterObjectProperty("jjPLAYER", "const int clientID", asOFFSET(jjPLAYER, clientID));
		engine->RegisterObjectProperty("jjPLAYER", "const int8 playerID", asOFFSET(jjPLAYER, playerID));
		engine->RegisterObjectProperty("jjPLAYER", "const int localPlayerID", asOFFSET(jjPLAYER, localPlayerID));
		engine->RegisterObjectProperty("jjPLAYER", "const bool teamRed", asOFFSET(jjPLAYER, team));
		engine->RegisterObjectProperty("jjPLAYER", "bool running", asOFFSET(jjPLAYER, run));
		engine->RegisterObjectProperty("jjPLAYER", "bool alreadyDoubleJumped", asOFFSET(jjPLAYER, specialJump)); // Deprecated
		engine->RegisterObjectProperty("jjPLAYER", "int doubleJumpCount", asOFFSET(jjPLAYER, specialJump));
		engine->RegisterObjectMethod("jjPLAYER", "int get_stoned() const", asMETHOD(jjPLAYER, get_stoned), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "int set_stoned(int)", asMETHOD(jjPLAYER, set_stoned), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjPLAYER", "int buttstomp", asOFFSET(jjPLAYER, buttstomp));
		engine->RegisterObjectProperty("jjPLAYER", "int helicopter", asOFFSET(jjPLAYER, helicopter));
		engine->RegisterObjectProperty("jjPLAYER", "int helicopterElapsed", asOFFSET(jjPLAYER, helicopterElapsed));
		engine->RegisterObjectProperty("jjPLAYER", "int specialMove", asOFFSET(jjPLAYER, specialMove));
		engine->RegisterObjectProperty("jjPLAYER", "int idle", asOFFSET(jjPLAYER, idle));
		engine->RegisterObjectMethod("jjPLAYER", "void suckerTube(int xSpeed, int ySpeed, bool center, bool noclip = false, bool trigSample = false)", asMETHOD(jjPLAYER, suckerTube), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "void poleSpin(float xSpeed, float ySpeed, uint delay = 70)", asMETHOD(jjPLAYER, poleSpin), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "void spring(float xSpeed, float ySpeed, bool keepZeroSpeeds, bool sample)", asMETHOD(jjPLAYER, spring), asCALL_THISCALL);

		engine->RegisterObjectProperty("jjPLAYER", "const bool isLocal", asOFFSET(jjPLAYER, isLocal));
		engine->RegisterObjectProperty("jjPLAYER", "const bool isActive", asOFFSET(jjPLAYER, isActive));
		engine->RegisterObjectMethod("jjPLAYER", "bool get_isConnecting() const", asMETHOD(jjPLAYER, get_isConnecting), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool get_isIdle() const", asMETHOD(jjPLAYER, get_isIdle), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool get_isOut() const", asMETHOD(jjPLAYER, get_isOut), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool get_isSpectating() const", asMETHOD(jjPLAYER, get_isSpectating), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool get_isInGame() const", asMETHOD(jjPLAYER, get_isInGame), asCALL_THISCALL);

		engine->RegisterObjectMethod("jjPLAYER", "string get_name() const", asMETHOD(jjPLAYER, get_name), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "string get_nameUnformatted() const", asMETHOD(jjPLAYER, get_nameUnformatted), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool setName(const string &in name)", asMETHOD(jjPLAYER, setName), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "int8 get_light() const", asMETHOD(jjPLAYER, get_light), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "int8 set_light(int8)", asMETHOD(jjPLAYER, set_light), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "uint32 get_fur() const", asMETHOD(jjPLAYER, get_fur), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "uint32 set_fur(uint32)", asMETHOD(jjPLAYER, set_fur), asCALL_THISCALL);
		// TODO
		/*engine->RegisterObjectMethod("jjPLAYER", "void furGet(uint8 &out a, uint8 &out b, uint8 &out c, uint8 &out d) const", asFUNCTION(getFur), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "void furSet(uint8 a, uint8 b, uint8 c, uint8 d)", asFUNCTION(setFur), asCALL_THISCALL);*/

		engine->RegisterObjectMethod("jjPLAYER", "bool get_noFire() const", asMETHOD(jjPLAYER, get_noFire), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool set_noFire(bool)", asMETHOD(jjPLAYER, set_noFire), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool get_antiGrav() const", asMETHOD(jjPLAYER, get_antiGrav), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool set_antiGrav(bool)", asMETHOD(jjPLAYER, set_antiGrav), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool get_invisibility() const", asMETHOD(jjPLAYER, get_invisibility), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool set_invisibility(bool)", asMETHOD(jjPLAYER, set_invisibility), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool get_noclipMode() const", asMETHOD(jjPLAYER, get_noclipMode), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool set_noclipMode(bool)", asMETHOD(jjPLAYER, set_noclipMode), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "uint8 get_lighting() const", asMETHOD(jjPLAYER, get_lighting), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "uint8 set_lighting(uint8)", asMETHOD(jjPLAYER, set_lighting), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "uint8 resetLight()", asMETHOD(jjPLAYER, resetLight), asCALL_THISCALL);

		engine->RegisterObjectMethod("jjPLAYER", "bool get_keyLeft() const", asMETHOD(jjPLAYER, get_playerKeyLeftPressed), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool get_keyRight() const", asMETHOD(jjPLAYER, get_playerKeyRightPressed), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool get_keyUp() const", asMETHOD(jjPLAYER, get_playerKeyUpPressed), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool get_keyDown() const", asMETHOD(jjPLAYER, get_playerKeyDownPressed), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool get_keyFire() const", asMETHOD(jjPLAYER, get_playerKeyFirePressed), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool get_keySelect() const", asMETHOD(jjPLAYER, get_playerKeySelectPressed), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool get_keyJump() const", asMETHOD(jjPLAYER, get_playerKeyJumpPressed), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool get_keyRun() const", asMETHOD(jjPLAYER, get_playerKeyRunPressed), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool set_keyLeft(bool)", asMETHOD(jjPLAYER, set_playerKeyLeftPressed), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool set_keyRight(bool)", asMETHOD(jjPLAYER, set_playerKeyRightPressed), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool set_keyUp(bool)", asMETHOD(jjPLAYER, set_playerKeyUpPressed), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool set_keyDown(bool)", asMETHOD(jjPLAYER, set_playerKeyDownPressed), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool set_keyFire(bool)", asMETHOD(jjPLAYER, set_playerKeyFirePressed), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool set_keySelect(bool)", asMETHOD(jjPLAYER, set_playerKeySelectPressed), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool set_keyJump(bool)", asMETHOD(jjPLAYER, set_playerKeyJumpPressed), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool set_keyRun(bool)", asMETHOD(jjPLAYER, set_playerKeyRunPressed), asCALL_THISCALL);

		engine->RegisterObjectMethod("jjPLAYER", "bool get_powerup(uint8) const", asMETHOD(jjPLAYER, get_powerup), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool set_powerup(uint8, bool)", asMETHOD(jjPLAYER, set_powerup), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "int get_ammo(uint8) const", asMETHOD(jjPLAYER, get_ammo), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "int set_ammo(uint8, int)", asMETHOD(jjPLAYER, set_ammo), asCALL_THISCALL);

		engine->RegisterObjectMethod("jjPLAYER", "bool offsetPosition(int xPixels, int yPixels)", asMETHOD(jjPLAYER, offsetPosition), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool warpToTile(int xTile, int yTile, bool fast = false)", asMETHOD(jjPLAYER, warpToTile), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool warpToID(uint8 warpID, bool fast = false)", asMETHOD(jjPLAYER, warpToID), asCALL_THISCALL);

		engine->SetDefaultNamespace("CHAR");
		engine->RegisterEnum("Char");
		engine->RegisterEnumValue("Char", "JAZZ", mJAZZ);
		engine->RegisterEnumValue("Char", "SPAZ", mSPAZ);
		engine->RegisterEnumValue("Char", "LORI", mLORI);

		engine->RegisterEnumValue("Char", "BIRD", mBIRD);
		engine->RegisterEnumValue("Char", "BIRD2", mCHUCK);
		engine->RegisterEnumValue("Char", "FROG", mFROG);
		engine->SetDefaultNamespace("");

		engine->RegisterObjectMethod("jjPLAYER", "CHAR::Char morph(bool rabbitsOnly = false, bool morphEffect = true)", asMETHOD(jjPLAYER, morph), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "CHAR::Char morphTo(CHAR::Char charNew, bool morphEffect = true)", asMETHOD(jjPLAYER, morphTo), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "CHAR::Char revertMorph(bool morphEffect = true)", asMETHOD(jjPLAYER, revertMorph), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "CHAR::Char get_charCurr() const", asMETHOD(jjPLAYER, get_charCurr), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjPLAYER", "CHAR::Char charOrig", asOFFSET(jjPLAYER, charOrig));

		engine->SetDefaultNamespace("TEAM");
		engine->RegisterEnum("Color");
		engine->RegisterEnumValue("Color", "NEUTRAL", -1);
		engine->RegisterEnumValue("Color", "BLUE", 0);
		engine->RegisterEnumValue("Color", "RED", 1);
		engine->RegisterEnumValue("Color", "GREEN", 2);
		engine->RegisterEnumValue("Color", "YELLOW", 3);
		engine->SetDefaultNamespace("");

		engine->SetDefaultNamespace("CHAT");
		engine->RegisterEnum("Type");
		engine->RegisterEnumValue("Type", "NORMAL", 0);
		engine->RegisterEnumValue("Type", "TEAMCHAT", 1);
		engine->RegisterEnumValue("Type", "WHISPER", 2);
		engine->RegisterEnumValue("Type", "ME", 3);
		engine->SetDefaultNamespace("");
		engine->RegisterObjectProperty("jjPLAYER", "const TEAM::Color team", asOFFSET(jjPLAYER, team));

		engine->RegisterObjectMethod("jjPLAYER", "void kill()", asMETHOD(jjPLAYER, kill), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool hurt(int8 damage = 1, bool forceHurt = false, jjPLAYER@ attacker = null)", asMETHOD(jjPLAYER, hurt), asCALL_THISCALL);

		// TODO
		//engine->RegisterGlobalFunction("array<jjPLAYER@>@ jjPlayersWithClientID(int clientID)", asFUNCTION(getPlayersWithClientID), asCALL_CDECL);*/

		engine->SetDefaultNamespace("TIMER");
		engine->RegisterEnum("State");
		engine->RegisterEnumValue("State", "STOPPED", 0);
		engine->RegisterEnumValue("State", "STARTED", 1);
		engine->RegisterEnumValue("State", "PAUSED", 2);
		engine->SetDefaultNamespace("STRING");
		engine->RegisterEnum("Mode");
		engine->RegisterEnumValue("Mode", "NORMAL", 0);
		engine->RegisterEnumValue("Mode", "DARK", 1);
		engine->RegisterEnumValue("Mode", "RIGHTALIGN", 2);
		engine->RegisterEnumValue("Mode", "BOUNCE", 3);
		engine->RegisterEnumValue("Mode", "SPIN", 4);
		engine->RegisterEnumValue("Mode", "PALSHIFT", 5);
		engine->RegisterEnum("Size");
		engine->RegisterEnumValue("Size", "SMALL", 1);
		engine->RegisterEnumValue("Size", "MEDIUM", 0);
		engine->RegisterEnumValue("Size", "LARGE", 2);
		engine->SetDefaultNamespace("");

		engine->RegisterGlobalFunction("void jjAlert(const ::string &in text, bool sendToAll = false, STRING::Size size = STRING::SMALL)", asFUNCTION(jjAlert), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjPrint(const ::string &in text, bool timestamp = false)", asFUNCTION(jjPrint), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjDebug(const ::string &in text, bool timestamp = false)", asFUNCTION(jjDebug), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjChat(const ::string &in text, bool teamchat = false)", asFUNCTION(jjChat), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjConsole(const ::string &in text, bool sendToAll = false)", asFUNCTION(jjConsole), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjSpy(const ::string &in text)", asFUNCTION(jjSpy), asCALL_CDECL);

		engine->RegisterObjectMethod("jjPLAYER", "TIMER::State get_timerState() const", asMETHOD(jjPLAYER, get_timerState), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool get_timerPersists() const", asMETHOD(jjPLAYER, get_timerPersists), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool set_timerPersists(bool)", asMETHOD(jjPLAYER, set_timerPersists), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "TIMER::State timerStart(int ticks, bool startPaused = false)", asMETHOD(jjPLAYER, timerStart), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "TIMER::State timerPause()", asMETHOD(jjPLAYER, timerPause), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "TIMER::State timerResume()", asMETHOD(jjPLAYER, timerResume), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "TIMER::State timerStop()", asMETHOD(jjPLAYER, timerStop), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "int get_timerTime() const", asMETHOD(jjPLAYER, get_timerTime), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "int set_timerTime(int)", asMETHOD(jjPLAYER, set_timerTime), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "void timerFunction(const string functionName)", asMETHOD(jjPLAYER, timerFunction), asCALL_THISCALL);
		engine->RegisterFuncdef("void jjVOIDFUNC()");
		engine->RegisterObjectMethod("jjPLAYER", "void timerFunction(jjVOIDFUNC@ function)", asMETHOD(jjPLAYER, timerFunctionPtr), asCALL_THISCALL);
		engine->RegisterFuncdef("void jjVOIDFUNCPLAYER(jjPLAYER@)");
		engine->RegisterObjectMethod("jjPLAYER", "void timerFunction(jjVOIDFUNCPLAYER@ function)", asMETHOD(jjPLAYER, timerFunctionFuncPtr), asCALL_THISCALL);

		engine->RegisterObjectMethod("jjPLAYER", "bool activateBoss(bool activate = true)", asMETHOD(jjPLAYER, activateBoss), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool limitXScroll(uint16 left, uint16 width)", asMETHOD(jjPLAYER, limitXScroll), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "void cameraFreeze(float xPixel, float yPixel, bool centered, bool instant)", asMETHOD(jjPLAYER, cameraFreezeFF), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "void cameraFreeze(bool xUnfreeze, float yPixel, bool centered, bool instant)", asMETHOD(jjPLAYER, cameraFreezeBF), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "void cameraFreeze(float xPixel, bool yUnfreeze, bool centered, bool instant)", asMETHOD(jjPLAYER, cameraFreezeFB), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "void cameraFreeze(bool xUnfreeze, bool yUnfreeze, bool centered, bool instant)", asMETHOD(jjPLAYER, cameraFreezeBB), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "void cameraUnfreeze(bool instant = true)", asMETHOD(jjPLAYER, cameraUnfreeze), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "void showText(string &in text, STRING::Size size = STRING::SMALL)", asMETHOD(jjPLAYER, showText), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "void showText(uint textID, uint offset, STRING::Size size = STRING::SMALL)", asMETHOD(jjPLAYER, showTextByID), asCALL_THISCALL);

		engine->SetDefaultNamespace("FLIGHT");
		engine->RegisterEnum("Mode");
		engine->RegisterEnumValue("Mode", "NONE", 0);
		engine->RegisterEnumValue("Mode", "FLYCARROT", 1);
		engine->RegisterEnumValue("Mode", "AIRBOARD", -1);
		engine->SetDefaultNamespace("");
		engine->RegisterObjectMethod("jjPLAYER", "FLIGHT::Mode get_fly() const", asMETHOD(jjPLAYER, get_fly), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "FLIGHT::Mode set_fly(FLIGHT::Mode)", asMETHOD(jjPLAYER, set_fly), asCALL_THISCALL);

		engine->SetDefaultNamespace("DIRECTION");
		engine->RegisterEnum("Dir");
		engine->RegisterEnumValue("Dir", "RIGHT", dirRIGHT);
		engine->RegisterEnumValue("Dir", "LEFT", dirLEFT);
		engine->RegisterEnumValue("Dir", "UP", dirUP);
		engine->RegisterEnumValue("Dir", "CURRENT", dirCURRENT);
		engine->SetDefaultNamespace("");

		engine->RegisterObjectMethod("jjPLAYER", "int fireBullet(uint8 gun = 0, bool depleteAmmo = true, bool requireAmmo = true, DIRECTION::Dir direction = DIRECTION::CURRENT)", asMETHOD(jjPLAYER, fireBulletDirection), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "int fireBullet(uint8 gun, bool depleteAmmo, bool requireAmmo, float angle)", asMETHOD(jjPLAYER, fireBulletAngle), asCALL_THISCALL);

		engine->RegisterObjectProperty("jjPLAYER", "const int subscreenX", asOFFSET(jjPLAYER, subscreenX));
		engine->RegisterObjectProperty("jjPLAYER", "const int subscreenY", asOFFSET(jjPLAYER, subscreenY));
		engine->RegisterObjectMethod("jjPLAYER", "float get_cameraX() const", asMETHOD(jjPLAYER, get_cameraX), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "float get_cameraY() const", asMETHOD(jjPLAYER, get_cameraY), asCALL_THISCALL);

		engine->RegisterObjectMethod("jjPLAYER", "int get_deaths() const", asMETHOD(jjPLAYER, get_deaths), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool get_isJailed() const", asMETHOD(jjPLAYER, get_isJailed), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool get_isZombie() const", asMETHOD(jjPLAYER, get_isZombie), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "int get_lrsLives() const", asMETHOD(jjPLAYER, get_lrsLives), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "int get_roasts() const", asMETHOD(jjPLAYER, get_roasts), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "int get_laps() const", asMETHOD(jjPLAYER, get_laps), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "int get_lapTimeCurrent() const", asMETHOD(jjPLAYER, get_lapTimeCurrent), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "int get_lapTimes(uint) const", asMETHOD(jjPLAYER, get_lapTimes), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "int get_lapTimeBest() const", asMETHOD(jjPLAYER, get_lapTimeBest), asCALL_THISCALL);

		engine->RegisterObjectMethod("jjPLAYER", "bool get_isAdmin() const", asMETHOD(jjPLAYER, get_isAdmin), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool hasPrivilege(const string &in privilege, uint moduleID = ::jjScriptModuleID) const", asMETHOD(jjPLAYER, hasPrivilege), asCALL_THISCALL);

		engine->RegisterGlobalProperty("const bool jjLowDetail", &parLowDetail);
		engine->RegisterGlobalProperty("const int jjColorDepth", &colorDepth);
		engine->RegisterGlobalProperty("const int jjResolutionMaxWidth", &checkedMaxSubVideoWidth);
		engine->RegisterGlobalProperty("const int jjResolutionMaxHeight", &checkedMaxSubVideoHeight);
		engine->RegisterGlobalProperty("const int jjResolutionWidth", &realVideoW);
		engine->RegisterGlobalProperty("const int jjResolutionHeight", &realVideoH);
		engine->RegisterGlobalProperty("const int jjSubscreenWidth", &subVideoW);
		engine->RegisterGlobalProperty("const int jjSubscreenHeight", &subVideoH);
		engine->RegisterGlobalFunction("int get_jjBorderWidth()", asFUNCTION(getBorderWidth), asCALL_CDECL);
		engine->RegisterGlobalFunction("int get_jjBorderHeight()", asFUNCTION(getBorderHeight), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool get_jjVerticalSplitscreen()", asFUNCTION(getSplitscreenType), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool set_jjVerticalSplitscreen(bool)", asFUNCTION(setSplitscreenType), asCALL_CDECL);

		// TODO
		/*engine->RegisterGlobalProperty("const bool jjAllowsFireball", &checkedFireball);
		engine->RegisterGlobalProperty("const bool jjAllowsMouseAim", &checkedAllowMouseAim);
		engine->RegisterGlobalProperty("const bool jjAllowsReady", &checkedAllowReady);
		engine->RegisterGlobalProperty("const bool jjAllowsWalljump", &checkedAllowWalljump);
		engine->RegisterGlobalProperty("const bool jjAlwaysRunning", &alwaysRunning);
		engine->RegisterGlobalProperty("const bool jjAutoWeaponChange", &weaponChange);
		engine->RegisterGlobalProperty("const bool jjFriendlyFire", &checkedFriendlyFire);
		engine->RegisterGlobalProperty("const bool jjMouseAim", &mouseAim);
		engine->RegisterGlobalProperty("const bool jjNoBlink", &checkedNoBlink);
		engine->RegisterGlobalProperty("const bool jjNoMovement", &checkedNoMovement);
		engine->RegisterGlobalProperty("const bool jjQuirks", &testQuirksMode);
		engine->RegisterGlobalProperty("const bool jjShowMaxHealth", &showEmptyHearts);
		engine->RegisterGlobalProperty("const bool jjStrongPowerups", &checkedStrongPowerups);*/

		engine->RegisterGlobalProperty("const int jjMaxScore", &maxScore);
		engine->RegisterGlobalFunction("int get_jjTeamScore(TEAM::Color)", asFUNCTION(get_teamScore), asCALL_CDECL);
		engine->RegisterGlobalFunction("int get_jjMaxHealth()", asFUNCTION(GetMaxHealth), asCALL_CDECL);
		engine->RegisterGlobalFunction("int get_jjStartHealth()", asFUNCTION(GetStartHealth), asCALL_CDECL);

		/*engine->RegisterGlobalProperty("const bool jjDoZombiesAlreadyExist", &doZombiesAlreadyExist);
		engine->RegisterGlobalFunction("jjPLAYER@ get_jjBottomFeeder()", asFUNCTION(get_bottomFeeder), asCALL_CDECL);
		engine->RegisterGlobalFunction("jjPLAYER@ get_jjTokenOwner()", asFUNCTION(get_tokenOwner), asCALL_CDECL);*/

		engine->RegisterGlobalFunction("float get_jjLayerXOffset(uint8)", asFUNCTION(get_layerXOffset), asCALL_CDECL);
		engine->RegisterGlobalFunction("float set_jjLayerXOffset(uint8, float)", asFUNCTION(set_layerXOffset), asCALL_CDECL);
		engine->RegisterGlobalFunction("float get_jjLayerYOffset(uint8)", asFUNCTION(get_layerYOffset), asCALL_CDECL);
		engine->RegisterGlobalFunction("float set_jjLayerYOffset(uint8, float)", asFUNCTION(set_layerYOffset), asCALL_CDECL);
		engine->RegisterGlobalFunction("int get_jjLayerWidth(uint8)", asFUNCTION(get_layerWidth), asCALL_CDECL);
		engine->RegisterGlobalFunction("int get_jjLayerWidthReal(uint8)", asFUNCTION(get_layerRealWidth), asCALL_CDECL);
		engine->RegisterGlobalFunction("int get_jjLayerWidthRounded(uint8)", asFUNCTION(get_layerRoundedWidth), asCALL_CDECL);
		engine->RegisterGlobalFunction("int get_jjLayerHeight(uint8)", asFUNCTION(get_layerHeight), asCALL_CDECL);
		engine->RegisterGlobalFunction("float get_jjLayerXSpeed(uint8)", asFUNCTION(get_layerXSpeed), asCALL_CDECL);
		engine->RegisterGlobalFunction("float set_jjLayerXSpeed(uint8, float)", asFUNCTION(set_layerXSpeed), asCALL_CDECL);
		engine->RegisterGlobalFunction("float get_jjLayerYSpeed(uint8)", asFUNCTION(get_layerYSpeed), asCALL_CDECL);
		engine->RegisterGlobalFunction("float set_jjLayerYSpeed(uint8, float)", asFUNCTION(set_layerYSpeed), asCALL_CDECL);
		engine->RegisterGlobalFunction("float get_jjLayerXAutoSpeed(uint8)", asFUNCTION(get_layerXAutoSpeed), asCALL_CDECL);
		engine->RegisterGlobalFunction("float set_jjLayerXAutoSpeed(uint8, float)", asFUNCTION(set_layerXAutoSpeed), asCALL_CDECL);
		engine->RegisterGlobalFunction("float get_jjLayerYAutoSpeed(uint8)", asFUNCTION(get_layerYAutoSpeed), asCALL_CDECL);
		engine->RegisterGlobalFunction("float set_jjLayerYAutoSpeed(uint8, float)", asFUNCTION(set_layerYAutoSpeed), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool get_jjLayerHasTiles(uint8)", asFUNCTION(get_layerHasTiles), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool set_jjLayerHasTiles(uint8, bool)", asFUNCTION(set_layerHasTiles), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool get_jjLayerTileHeight(uint8)", asFUNCTION(get_layerTileHeight), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool set_jjLayerTileHeight(uint8, bool)", asFUNCTION(set_layerTileHeight), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool get_jjLayerTileWidth(uint8)", asFUNCTION(get_layerTileWidth), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool set_jjLayerTileWidth(uint8, bool)", asFUNCTION(set_layerTileWidth), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool get_jjLayerLimitVisibleRegion(uint8)", asFUNCTION(get_layerLimitVisibleRegion), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool set_jjLayerLimitVisibleRegion(uint8, bool)", asFUNCTION(set_layerLimitVisibleRegion), asCALL_CDECL);

		engine->RegisterGlobalFunction("void jjSetLayerXSpeed(uint8 layerID, float newspeed, bool newSpeedIsAnAutoSpeed)", asFUNCTION(setLayerXSpeedSeamlessly), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjSetLayerYSpeed(uint8 layerID, float newspeed, bool newSpeedIsAnAutoSpeed)", asFUNCTION(setLayerYSpeedSeamlessly), asCALL_CDECL);

		engine->RegisterObjectType("jjPALCOLOR", sizeof(jjPALCOLOR), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<jjPALCOLOR>());
		engine->RegisterObjectBehaviour("jjPALCOLOR", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(jjPALCOLOR::Create), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("jjPALCOLOR", asBEHAVE_CONSTRUCT, "void f(uint8 red, uint8 green, uint8 blue)", asFUNCTION(jjPALCOLOR::CreateFromRgb), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectProperty("jjPALCOLOR", "uint8 red", asOFFSET(jjPALCOLOR, red));
		engine->RegisterObjectProperty("jjPALCOLOR", "uint8 green", asOFFSET(jjPALCOLOR, green));
		engine->RegisterObjectProperty("jjPALCOLOR", "uint8 blue", asOFFSET(jjPALCOLOR, blue));
		engine->RegisterObjectMethod("jjPALCOLOR", "jjPALCOLOR& opAssign(const jjPALCOLOR &in)", asMETHODPR(jjPALCOLOR, operator=, (const jjPALCOLOR&), jjPALCOLOR&), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPALCOLOR", "bool opEquals(const jjPALCOLOR &in) const", asMETHOD(jjPALCOLOR, operator==), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPALCOLOR", "uint8 getHue() const", asMETHOD(jjPALCOLOR, getHue), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPALCOLOR", "uint8 getSat() const", asMETHOD(jjPALCOLOR, getSat), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPALCOLOR", "uint8 getLight() const", asMETHOD(jjPALCOLOR, getLight), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPALCOLOR", "void setHSL(int hue, uint8 sat, uint8 light)", asMETHOD(jjPALCOLOR, setHSL), asCALL_THISCALL);
		engine->SetDefaultNamespace("COLOR");
		engine->RegisterEnum("Component");
		engine->RegisterEnumValue("Component", "RED", 0);
		engine->RegisterEnumValue("Component", "GREEN", 1);
		engine->RegisterEnumValue("Component", "BLUE", 2);
		engine->SetDefaultNamespace("");
		engine->RegisterObjectMethod("jjPALCOLOR", "void swizzle(COLOR::Component red, COLOR::Component green, COLOR::Component blue)", asMETHOD(jjPALCOLOR, swizzle), asCALL_THISCALL);

		engine->RegisterObjectType("jjPAL", sizeof(jjPAL), asOBJ_REF);
		engine->RegisterObjectBehaviour("jjPAL", asBEHAVE_FACTORY, "jjPAL@ f()", asFUNCTION(jjPAL::Create), asCALL_CDECL);
		engine->RegisterObjectBehaviour("jjPAL", asBEHAVE_ADDREF, "void f()", asMETHOD(jjPAL, AddRef), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("jjPAL", asBEHAVE_RELEASE, "void f()", asMETHOD(jjPAL, Release), asCALL_THISCALL);
		engine->RegisterGlobalProperty("jjPAL jjPalette", &jjPalette);
		engine->RegisterGlobalProperty("const jjPAL jjBackupPalette", &jjBackupPalette);
		// TODO
		/*engine->RegisterObjectMethod("jjPAL", "jjPAL& opAssign(const jjPAL &in)", asMETHOD(Tpalette, operator=), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPAL", "bool opEquals(const jjPAL &in) const", asMETHOD(Tpalette, operator==), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPAL", "jjPALCOLOR& get_color(uint8)", asMETHOD(Tpalette, getColor), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPAL", "const jjPALCOLOR& get_color(uint8) const", asMETHOD(Tpalette, getConstColor), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPAL", "jjPALCOLOR& set_color(uint8, const jjPALCOLOR &in)", asMETHOD(Tpalette, setColorEntry), asCALL_THISCALL);*/
		engine->RegisterObjectMethod("jjPAL", "void reset()", asMETHOD(jjPAL, reset), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPAL", "void apply() const", asMETHOD(jjPAL, apply), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPAL", "bool load(string &in filename)", asMETHOD(jjPAL, load), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPAL", "void fill(uint8 red, uint8 green, uint8 blue, float opacity)", asMETHOD(jjPAL, fill), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPAL", "void fill(uint8 red, uint8 green, uint8 blue, uint8 start = 1, uint8 length = 254, float opacity = 1.0)", asMETHOD(jjPAL, fillTint), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPAL", "void fill(jjPALCOLOR color, float opacity)", asMETHOD(jjPAL, fillFromColor), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPAL", "void fill(jjPALCOLOR color, uint8 start = 1, uint8 length = 254, float opacity = 1.0)", asMETHOD(jjPAL, fillTintFromColor), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPAL", "void gradient(uint8 red1, uint8 green1, uint8 blue1, uint8 red2, uint8 green2, uint8 blue2, uint8 start = 176, uint8 length = 32, float opacity = 1.0, bool inclusive = false)", asMETHOD(jjPAL, gradient), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPAL", "void gradient(jjPALCOLOR color1, jjPALCOLOR color2, uint8 start = 176, uint8 length = 32, float opacity = 1.0, bool inclusive = false)", asMETHOD(jjPAL, gradientFromColor), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPAL", "void copyFrom(uint8 start, uint8 length, uint8 start2, const jjPAL &in source, float opacity = 1.0)", asMETHOD(jjPAL, copyFrom), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPAL", "uint8 findNearestColor(jjPALCOLOR color) const", asMETHOD(jjPAL, findNearestColor), asCALL_THISCALL);

		engine->SetDefaultNamespace("SPRITE");
		engine->RegisterEnum("Mode");
		engine->RegisterEnumValue("Mode", "NORMAL", spriteType_NORMAL);
		engine->RegisterEnumValue("Mode", "TRANSLUCENT", spriteType_TRANSLUCENT);
		engine->RegisterEnumValue("Mode", "TINTED", spriteType_TINTED);
		engine->RegisterEnumValue("Mode", "GEM", spriteType_GEM);
		engine->RegisterEnumValue("Mode", "INVISIBLE", spriteType_INVISIBLE);
		engine->RegisterEnumValue("Mode", "SINGLECOLOR", spriteType_SINGLECOLOR);
		engine->RegisterEnumValue("Mode", "RESIZED", spriteType_RESIZED);
		engine->RegisterEnumValue("Mode", "NEONGLOW", spriteType_NEONGLOW);
		engine->RegisterEnumValue("Mode", "FROZEN", spriteType_FROZEN);
		engine->RegisterEnumValue("Mode", "PLAYER", spriteType_PLAYER);
		engine->RegisterEnumValue("Mode", "PALSHIFT", spriteType_PALSHIFT);
		engine->RegisterEnumValue("Mode", "SHADOW", spriteType_SHADOW);
		engine->RegisterEnumValue("Mode", "SINGLEHUE", spriteType_SINGLEHUE);
		engine->RegisterEnumValue("Mode", "BRIGHTNESS", spriteType_BRIGHTNESS);
		engine->RegisterEnumValue("Mode", "TRANSLUCENTCOLOR", spriteType_TRANSLUCENTCOLOR);
		engine->RegisterEnumValue("Mode", "TRANSLUCENTPLAYER", spriteType_TRANSLUCENTPLAYER);
		engine->RegisterEnumValue("Mode", "TRANSLUCENTPALSHIFT", spriteType_TRANSLUCENTPALSHIFT);
		engine->RegisterEnumValue("Mode", "TRANSLUCENTSINGLEHUE", spriteType_TRANSLUCENTSINGLEHUE);
		engine->RegisterEnumValue("Mode", "ALPHAMAP", spriteType_ALPHAMAP);
		engine->RegisterEnumValue("Mode", "MENUPLAYER", spriteType_MENUPLAYER);
		engine->RegisterEnumValue("Mode", "BLEND_NORMAL", spriteType_BLENDNORMAL);
		engine->RegisterEnumValue("Mode", "BLEND_DARKEN", spriteType_BLENDDARKEN);
		engine->RegisterEnumValue("Mode", "BLEND_LIGHTEN", spriteType_BLENDLIGHTEN);
		engine->RegisterEnumValue("Mode", "BLEND_HUE", spriteType_BLENDHUE);
		engine->RegisterEnumValue("Mode", "BLEND_SATURATION", spriteType_BLENDSATURATION);
		engine->RegisterEnumValue("Mode", "BLEND_COLOR", spriteType_BLENDCOLOR);
		engine->RegisterEnumValue("Mode", "BLEND_LUMINANCE", spriteType_BLENDLUMINANCE);
		engine->RegisterEnumValue("Mode", "BLEND_MULTIPLY", spriteType_BLENDMULTIPLY);
		engine->RegisterEnumValue("Mode", "BLEND_SCREEN", spriteType_BLENDSCREEN);
		engine->RegisterEnumValue("Mode", "BLEND_DISSOLVE", spriteType_BLENDDISSOLVE);
		engine->RegisterEnumValue("Mode", "BLEND_OVERLAY", spriteType_BLENDOVERLAY);
		engine->RegisterEnumValue("Mode", "BLEND_HARDLIGHT", spriteType_BLENDHARDLIGHT);
		engine->RegisterEnumValue("Mode", "BLEND_SOFTLIGHT", spriteType_BLENDSOFTLIGHT);
		engine->RegisterEnumValue("Mode", "BLEND_DIFFERENCE", spriteType_BLENDDIFFERENCE);
		engine->RegisterEnumValue("Mode", "BLEND_DODGE", spriteType_BLENDDODGE);
		engine->RegisterEnumValue("Mode", "BLEND_BURN", spriteType_BLENDBURN);
		engine->RegisterEnumValue("Mode", "BLEND_EXCLUSION", spriteType_BLENDEXCLUSION);
		engine->RegisterEnumValue("Mode", "TRANSLUCENTTILE", spriteType_TRANSLUCENTTILE);
		engine->RegisterEnumValue("Mode", "CHROMAKEY", spriteType_CHROMAKEY);
		engine->RegisterEnumValue("Mode", "MAPPING", spriteType_MAPPING);
		engine->RegisterEnumValue("Mode", "TRANSLUCENTMAPPING", spriteType_TRANSLUCENTMAPPING);
		engine->RegisterEnum("Direction");
		engine->RegisterEnumValue("Direction", "FLIPNONE", 0x00);
		engine->RegisterEnumValue("Direction", "FLIPH", 0xFF - 0x100);
		engine->RegisterEnumValue("Direction", "FLIPV", 0x40);
		engine->RegisterEnumValue("Direction", "FLIPHV", 0xBF - 0x100);
		engine->SetDefaultNamespace("TILE");
		engine->RegisterEnum("Quadrant");
		engine->RegisterEnumValue("Quadrant", "TOPLEFT", 0);
		engine->RegisterEnumValue("Quadrant", "TOPRIGHT", 1);
		engine->RegisterEnumValue("Quadrant", "BOTTOMLEFT", 2);
		engine->RegisterEnumValue("Quadrant", "BOTTOMRIGHT", 3);
		engine->RegisterEnumValue("Quadrant", "ALLQUADRANTS", 4);
		engine->RegisterEnum("Flags");
		engine->RegisterEnumValue("Flags", "RAWRANGE", FLAG_HFLIPPED_TILE - 1);
		engine->RegisterEnumValue("Flags", "HFLIPPED", FLAG_HFLIPPED_TILE);
		engine->RegisterEnumValue("Flags", "VFLIPPED", FLAG_VFLIPPED_TILE);
		engine->RegisterEnumValue("Flags", "ANIMATED", FLAG_ANIMATED_TILE);
		engine->SetDefaultNamespace("");

		// TODO
		/*engine->RegisterObjectMethod("jjPLAYER", "SPRITE::Mode get_spriteMode() const", asFUNCTION(getSpriteMode), asCALL_CDECL_OBJFIRST);
		engine->RegisterObjectMethod("jjPLAYER", "SPRITE::Mode set_spriteMode(SPRITE::Mode)", asFUNCTION(setSpriteMode), asCALL_CDECL_OBJFIRST);
		engine->RegisterObjectMethod("jjPLAYER", "uint8 get_spriteParam() const", asFUNCTION(getSpriteParam), asCALL_CDECL_OBJFIRST);
		engine->RegisterObjectMethod("jjPLAYER", "uint8 set_spriteParam(uint8)", asFUNCTION(setSpriteParam), asCALL_CDECL_OBJFIRST);

		engine->RegisterGlobalFunction("void jjSpriteModeSetMapping(uint8 index, const array<uint8> &in indexMapping, const jjPAL &in rgbMapping)", asFUNCTION(setSpriteModeMapping), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjSpriteModeSetMapping(uint8 index, const array<uint8> &in indexMapping)", asFUNCTION(setSpriteModeMappingDynamic), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool jjSpriteModeIsMappingUsed(uint8 index)", asFUNCTION(isSpriteModeMappingUsed), asCALL_CDECL);
		engine->RegisterGlobalFunction("int jjSpriteModeFirstFreeMapping()", asFUNCTION(firstFreeSpriteModeMapping), asCALL_CDECL);
		engine->RegisterGlobalFunction("array<uint8>@ jjSpriteModeGetIndexMapping(uint8 index)", asFUNCTION(getSpriteModeIndexMapping), asCALL_CDECL);
		engine->RegisterGlobalFunction("jjPAL@ jjSpriteModeGetColorMapping(uint8 index)", asFUNCTION(getSpriteModeColorMapping), asCALL_CDECL);*/

		engine->RegisterObjectType("jjTEXTAPPEARANCE", sizeof(jjTEXTAPPEARANCE), asOBJ_VALUE | asOBJ_POD);
		engine->RegisterObjectBehaviour("jjTEXTAPPEARANCE", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(jjTEXTAPPEARANCE::constructor), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("jjTEXTAPPEARANCE", asBEHAVE_CONSTRUCT, "void f(STRING::Mode mode)", asFUNCTION(jjTEXTAPPEARANCE::constructorMode), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectMethod("jjTEXTAPPEARANCE", "jjTEXTAPPEARANCE& opAssign(STRING::Mode)", asMETHODPR(jjTEXTAPPEARANCE, operator=, (uint32_t), jjTEXTAPPEARANCE&), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjTEXTAPPEARANCE", "int xAmp", asOFFSET(jjTEXTAPPEARANCE, xAmp));
		engine->RegisterObjectProperty("jjTEXTAPPEARANCE", "int yAmp", asOFFSET(jjTEXTAPPEARANCE, yAmp));
		engine->RegisterObjectProperty("jjTEXTAPPEARANCE", "int spacing", asOFFSET(jjTEXTAPPEARANCE, spacing));
		engine->RegisterObjectProperty("jjTEXTAPPEARANCE", "bool monospace", asOFFSET(jjTEXTAPPEARANCE, monospace));
		engine->RegisterObjectProperty("jjTEXTAPPEARANCE", "bool skipInitialHash", asOFFSET(jjTEXTAPPEARANCE, skipInitialHash));
		engine->SetDefaultNamespace("STRING");
		engine->RegisterEnum("SignTreatment");
		engine->RegisterEnumValue("SignTreatment", "HIDESIGN", jjTEXTAPPEARANCE::ch_HIDE);
		engine->RegisterEnumValue("SignTreatment", "DISPLAYSIGN", jjTEXTAPPEARANCE::ch_DISPLAY);
		engine->RegisterEnumValue("SignTreatment", "SPECIALSIGN", jjTEXTAPPEARANCE::ch_SPECIAL);
		engine->RegisterEnum("Alignment");
		engine->RegisterEnumValue("Alignment", "DEFAULT", jjTEXTAPPEARANCE::align_DEFAULT);
		engine->RegisterEnumValue("Alignment", "LEFT", jjTEXTAPPEARANCE::align_LEFT);
		engine->RegisterEnumValue("Alignment", "CENTER", jjTEXTAPPEARANCE::align_CENTER);
		engine->RegisterEnumValue("Alignment", "RIGHT", jjTEXTAPPEARANCE::align_RIGHT);
		engine->SetDefaultNamespace("");
		engine->RegisterObjectProperty("jjTEXTAPPEARANCE", "STRING::SignTreatment at", asOFFSET(jjTEXTAPPEARANCE, at));
		engine->RegisterObjectProperty("jjTEXTAPPEARANCE", "STRING::SignTreatment caret", asOFFSET(jjTEXTAPPEARANCE, caret));
		engine->RegisterObjectProperty("jjTEXTAPPEARANCE", "STRING::SignTreatment hash", asOFFSET(jjTEXTAPPEARANCE, hash));
		engine->RegisterObjectProperty("jjTEXTAPPEARANCE", "STRING::SignTreatment newline", asOFFSET(jjTEXTAPPEARANCE, newline));
		engine->RegisterObjectProperty("jjTEXTAPPEARANCE", "STRING::SignTreatment pipe", asOFFSET(jjTEXTAPPEARANCE, pipe));
		engine->RegisterObjectProperty("jjTEXTAPPEARANCE", "STRING::SignTreatment section", asOFFSET(jjTEXTAPPEARANCE, section));
		engine->RegisterObjectProperty("jjTEXTAPPEARANCE", "STRING::SignTreatment tilde", asOFFSET(jjTEXTAPPEARANCE, tilde));
		engine->RegisterObjectProperty("jjTEXTAPPEARANCE", "STRING::Alignment align", asOFFSET(jjTEXTAPPEARANCE, align));

		engine->RegisterObjectType("jjCANVAS", sizeof(jjCANVAS), asOBJ_REF | asOBJ_NOCOUNT);
		engine->RegisterObjectMethod("jjCANVAS", "void drawPixel(int xPixel, int yPixel, uint8 color, SPRITE::Mode mode = SPRITE::NORMAL, uint8 param = 0)", asMETHOD(jjCANVAS, DrawPixel), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjCANVAS", "void drawRectangle(int xPixel, int yPixel, int width, int height, uint8 color, SPRITE::Mode mode = SPRITE::NORMAL, uint8 param = 0)", asMETHOD(jjCANVAS, DrawRectangle), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjCANVAS", "void drawSprite(int xPixel, int yPixel, int setID, uint8 animation, uint8 frame, int8 direction = 0, SPRITE::Mode mode = SPRITE::NORMAL, uint8 param = 0)", asMETHOD(jjCANVAS, DrawSprite), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjCANVAS", "void drawSpriteFromCurFrame(int xPixel, int yPixel, uint sprite, int8 direction = 0, SPRITE::Mode mode = SPRITE::NORMAL, uint8 param = 0)", asMETHOD(jjCANVAS, DrawCurFrameSprite), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjCANVAS", "void drawResizedSprite(int xPixel, int yPixel, int setID, uint8 animation, uint8 frame, float xScale, float yScale, SPRITE::Mode mode = SPRITE::NORMAL, uint8 param = 0)", asMETHOD(jjCANVAS, DrawResizedSprite), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjCANVAS", "void drawResizedSpriteFromCurFrame(int xPixel, int yPixel, uint sprite, float xScale, float yScale, SPRITE::Mode mode = SPRITE::NORMAL, uint8 param = 0)", asMETHOD(jjCANVAS, DrawResizedCurFrameSprite), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjCANVAS", "void drawRotatedSprite(int xPixel, int yPixel, int setID, uint8 animation, uint8 frame, int angle, float xScale = 1, float yScale = 1, SPRITE::Mode mode = SPRITE::NORMAL, uint8 param = 0)", asMETHOD(jjCANVAS, DrawTransformedSprite), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjCANVAS", "void drawRotatedSpriteFromCurFrame(int xPixel, int yPixel, uint sprite, int angle, float xScale = 1, float yScale = 1, SPRITE::Mode mode = SPRITE::NORMAL, uint8 param = 0)", asMETHOD(jjCANVAS, DrawTransformedCurFrameSprite), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjCANVAS", "void drawSwingingVineSpriteFromCurFrame(int xPixel, int yPixel, uint sprite, int length, int curvature, SPRITE::Mode mode = SPRITE::NORMAL, uint8 param = 0)", asMETHOD(jjCANVAS, DrawSwingingVine), asCALL_THISCALL);

		engine->RegisterObjectMethod("jjCANVAS", "void drawTile(int xPixel, int yPixel, uint16 tile, TILE::Quadrant tileQuadrant = TILE::ALLQUADRANTS)", asMETHOD(jjCANVAS, ExternalDrawTile), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjCANVAS", "void drawString(int xPixel, int yPixel, const ::string &in text, STRING::Size size = STRING::SMALL, STRING::Mode mode = STRING::NORMAL, uint8 param = 0)", asMETHOD(jjCANVAS, DrawTextBasicSize), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjCANVAS", "void drawString(int xPixel, int yPixel, const ::string &in text, STRING::Size size, const jjTEXTAPPEARANCE &in appearance, uint8 param1 = 0, SPRITE::Mode spriteMode = SPRITE::PALSHIFT, uint8 param2 = 0)", asMETHOD(jjCANVAS, DrawTextExtSize), asCALL_THISCALL);

		engine->RegisterGlobalFunction("void jjDrawPixel(float xPixel, float yPixel, uint8 color, SPRITE::Mode mode = SPRITE::NORMAL, uint8 param = 0, int8 layerZ = 4, uint8 layerXY = 4, int8 playerID = -1)", asFUNCTION(jjDrawPixel), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjDrawRectangle(float xPixel, float yPixel, int width, int height, uint8 color, SPRITE::Mode mode = SPRITE::NORMAL, uint8 param = 0, int8 layerZ = 4, uint8 layerXY = 4, int8 playerID = -1)", asFUNCTION(jjDrawRectangle), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjDrawSprite(float xPixel, float yPixel, int setID, uint8 animation, uint8 frame, int8 direction = 0, SPRITE::Mode mode = SPRITE::NORMAL, uint8 param = 0, int8 layerZ = 4, uint8 layerXY = 4, int8 playerID = -1)", asFUNCTION(jjDrawSprite), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjDrawSpriteFromCurFrame(float xPixel, float yPixel, uint sprite, int8 direction = 0, SPRITE::Mode mode = SPRITE::NORMAL, uint8 param = 0, int8 layerZ = 4, uint8 layerXY = 4, int8 playerID = -1)", asFUNCTION(jjDrawSpriteFromCurFrame), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjDrawResizedSprite(float xPixel, float yPixel, int setID, uint8 animation, uint8 frame, float xScale, float yScale, SPRITE::Mode mode = SPRITE::NORMAL, uint8 param = 0, int8 layerZ = 4, uint8 layerXY = 4, int8 playerID = -1)", asFUNCTION(jjDrawResizedSprite), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjDrawResizedSpriteFromCurFrame(float xPixel, float yPixel, uint sprite, float xScale, float yScale, SPRITE::Mode mode = SPRITE::NORMAL, uint8 param = 0, int8 layerZ = 4, uint8 layerXY = 4, int8 playerID = -1)", asFUNCTION(jjDrawResizedSpriteFromCurFrame), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjDrawRotatedSprite(float xPixel, float yPixel, int setID, uint8 animation, uint8 frame, int angle, float xScale = 1, float yScale = 1, SPRITE::Mode mode = SPRITE::NORMAL, uint8 param = 0, int8 layerZ = 4, uint8 layerXY = 4, int8 playerID = -1)", asFUNCTION(jjDrawRotatedSprite), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjDrawRotatedSpriteFromCurFrame(float xPixel, float yPixel, uint sprite, int angle, float xScale = 1, float yScale = 1, SPRITE::Mode mode = SPRITE::NORMAL, uint8 param = 0, int8 layerZ = 4, uint8 layerXY = 4, int8 playerID = -1)", asFUNCTION(jjDrawRotatedSpriteFromCurFrame), asCALL_CDECL);

		engine->RegisterGlobalFunction("void jjDrawSwingingVineSpriteFromCurFrame(float xPixel, float yPixel, uint sprite, int length, int curvature, SPRITE::Mode mode = SPRITE::NORMAL, uint8 param = 0, int8 layerZ = 4, uint8 layerXY = 4, int8 playerID = -1)", asFUNCTION(jjDrawSwingingVineSpriteFromCurFrame), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjDrawTile(float xPixel, float yPixel, uint16 tile, TILE::Quadrant tileQuadrant = TILE::ALLQUADRANTS, int8 layerZ = 4, uint8 layerXY = 4, int8 playerID = -1)", asFUNCTION(jjDrawTile), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjDrawString(float xPixel, float yPixel, const ::string &in text, STRING::Size size = STRING::SMALL, STRING::Mode mode = STRING::NORMAL, uint8 param = 0, int8 layerZ = 4, uint8 layerXY = 4, int8 playerID = -1)", asFUNCTION(jjDrawString), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjDrawString(float xPixel, float yPixel, const ::string &in text, STRING::Size size, const jjTEXTAPPEARANCE &in appearance, uint8 param1 = 0, SPRITE::Mode spriteMode = SPRITE::PALSHIFT, uint8 param2 = 0, int8 layerZ = 4, uint8 layerXY = 4, int8 playerID = -1)", asFUNCTION(jjDrawStringEx), asCALL_CDECL);
		engine->RegisterGlobalFunction("int jjGetStringWidth(const ::string &in text, STRING::Size size, const jjTEXTAPPEARANCE &in style)", asFUNCTION(jjGetStringWidth), asCALL_CDECL);

		engine->SetDefaultNamespace("TEXTURE");
		engine->RegisterEnum("Texture");
		engine->RegisterEnumValue("Texture", "FROMTILES", 0);
		engine->RegisterEnumValue("Texture", "LAYER8", 0);
		engine->RegisterEnumValue("Texture", "NORMAL", 1);
		engine->RegisterEnumValue("Texture", "PSYCH", 2);
		engine->RegisterEnumValue("Texture", "MEDIVO", 3);
		engine->RegisterEnumValue("Texture", "DIAMONDUSBETA", 4);
		engine->RegisterEnumValue("Texture", "WISETYNESS", 5);
		engine->RegisterEnumValue("Texture", "BLADE", 6);
		engine->RegisterEnumValue("Texture", "MEZ02", 7);
		engine->RegisterEnumValue("Texture", "WINDSTORMFORTRESS", 8);
		engine->RegisterEnumValue("Texture", "RANEFORUSV", 9);
		engine->RegisterEnumValue("Texture", "CORRUPTEDSANCTUARY", 10);
		engine->RegisterEnumValue("Texture", "XARGON", 11);
		engine->RegisterEnumValue("Texture", "ICTUBELECTRIC", 12);
		engine->RegisterEnumValue("Texture", "WTF", 13);
		engine->RegisterEnumValue("Texture", "MUCKAMOKNIGHT", 14);
		engine->RegisterEnumValue("Texture", "DESOLATION", 15);
		engine->RegisterEnumValue("Texture", "CUSTOM", ~0);
		engine->RegisterEnum("Style");
		engine->RegisterEnumValue("Style", "WARPHORIZON", tbgModeWARPHORIZON);
		engine->RegisterEnumValue("Style", "TUNNEL", tbgModeTUNNEL);
		engine->RegisterEnumValue("Style", "MENU", tbgModeMENU);
		engine->RegisterEnumValue("Style", "TILEMENU", tbgModeTILEMENU);
		engine->RegisterEnumValue("Style", "WAVE", tbgModeWAVE);
		engine->RegisterEnumValue("Style", "CYLINDER", tbgModeCYLINDER);
		engine->RegisterEnumValue("Style", "REFLECTION", tbgModeREFLECTION);
		engine->SetDefaultNamespace("");

		// TODO
		/*engine->RegisterGlobalFunction("void jjSetDarknessColor(jjPALCOLOR color = jjPALCOLOR())", asFUNCTION(jjSetDarknessColor), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjSetFadeColors(uint8 red, uint8 green, uint8 blue)", asFUNCTION(jjSetFadeColors), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjSetFadeColors(uint8 paletteColorID = 207)", asFUNCTION(jjSetFadeColorsFromPalette), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjSetFadeColors(jjPALCOLOR color)", asFUNCTION(jjSetFadeColorsFromPalcolor), asCALL_CDECL);
		engine->RegisterGlobalFunction("jjPALCOLOR jjGetFadeColors()", asFUNCTION(jjGetFadeColors), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjUpdateTexturedBG()", asFUNCTION(jjUpdateTexturedBG), asCALL_CDECL); // Deprecated
		engine->RegisterGlobalFunction("TEXTURE::Texture get_jjTexturedBGTexture()", asFUNCTION(get_jjTexturedBGTexture), asCALL_CDECL);
		engine->RegisterGlobalFunction("TEXTURE::Texture set_jjTexturedBGTexture(TEXTURE::Texture)", asFUNCTION(set_jjTexturedBGTexture), asCALL_CDECL);
		engine->RegisterGlobalFunction("TEXTURE::Style get_jjTexturedBGStyle()", asFUNCTION(get_jjTexturedBGStyle), asCALL_CDECL);
		engine->RegisterGlobalFunction("TEXTURE::Style set_jjTexturedBGStyle(TEXTURE::Style)", asFUNCTION(set_jjTexturedBGStyle), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool get_jjTexturedBGUsed()", asFUNCTION(get_jjTexturedBGUsed), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool set_jjTexturedBGUsed(bool)", asFUNCTION(set_jjTexturedBGUsed), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool get_jjTexturedBGStars()", asFUNCTION(get_jjTexturedBGStars), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool set_jjTexturedBGStars(bool)", asFUNCTION(set_jjTexturedBGStars), asCALL_CDECL);
		engine->RegisterGlobalProperty("float jjTexturedBGFadePositionX", &(BackgroundLayer.WARPHORIZON.FadePosition[0]));
		engine->RegisterGlobalProperty("float jjTexturedBGFadePositionY", &(BackgroundLayer.WARPHORIZON.FadePosition[1]));*/

		engine->SetDefaultNamespace("SNOWING");
		engine->RegisterEnum("Type");
		engine->RegisterEnumValue("Type", "SNOW", 0);
		engine->RegisterEnumValue("Type", "FLOWER", 1);
		engine->RegisterEnumValue("Type", "RAIN", 2);
		engine->RegisterEnumValue("Type", "LEAF", 3);
		engine->SetDefaultNamespace("");
		engine->RegisterGlobalProperty("bool jjIsSnowing", &snowing);
		engine->RegisterGlobalProperty("bool jjIsSnowingOutdoorsOnly", &snowingOutdoors);
		engine->RegisterGlobalProperty("uint8 jjSnowingIntensity", &snowingIntensity);
		engine->RegisterGlobalProperty("SNOWING::Type jjSnowingType", &snowingType);

		engine->RegisterGlobalFunction("bool get_jjTriggers(uint8)", asFUNCTION(get_jjTriggers), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool set_jjTriggers(uint8, bool)", asFUNCTION(set_jjTriggers), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool jjSwitchTrigger(uint8 id)", asFUNCTION(jjSwitchTrigger), asCALL_CDECL);

		engine->RegisterGlobalFunction("bool get_jjEnabledASFunctions(uint8)", asFUNCTION(isNumberedASFunctionEnabled), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool set_jjEnabledASFunctions(uint8, bool)", asFUNCTION(setNumberedASFunctionEnabled), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjEnableEachASFunction()", asFUNCTION(reenableAllNumberedASFunctions), asCALL_CDECL);

		engine->SetDefaultNamespace("WATERLIGHT");
		engine->RegisterEnum("wl");
		engine->RegisterEnumValue("wl", "NONE", 0);
		engine->RegisterEnumValue("wl", "GLOBAL", 1);
		engine->RegisterEnumValue("wl", "LAGUNICUS", 3);
		engine->SetDefaultNamespace("WATERINTERACTION");
		engine->RegisterEnum("WaterInteraction");
		engine->RegisterEnumValue("WaterInteraction", "POSITIONBASED", waterInteraction_POSITIONBASED);
		engine->RegisterEnumValue("WaterInteraction", "SWIM", waterInteraction_SWIM);
		engine->RegisterEnumValue("WaterInteraction", "LOWGRAVITY", waterInteraction_LOWGRAVITY);
		engine->SetDefaultNamespace("");
		engine->RegisterGlobalProperty("WATERLIGHT::wl jjWaterLighting", &waterLightMode);
		engine->RegisterGlobalProperty("WATERINTERACTION::WaterInteraction jjWaterInteraction", &waterInteraction);
		engine->RegisterGlobalFunction("float get_jjWaterLevel()", asFUNCTION(getWaterLevel), asCALL_CDECL);
		engine->RegisterGlobalFunction("float get_jjWaterTarget()", asFUNCTION(getWaterLevel2), asCALL_CDECL);
		engine->RegisterGlobalFunction("float jjSetWaterLevel(float yPixel, bool instant)", asFUNCTION(setWaterLevel), asCALL_CDECL);
		engine->RegisterGlobalFunction("float get_jjWaterChangeSpeed()", asFUNCTION(get_waterChangeSpeed), asCALL_CDECL);
		engine->RegisterGlobalFunction("float set_jjWaterChangeSpeed(float)", asFUNCTION(set_waterChangeSpeed), asCALL_CDECL);
		engine->RegisterGlobalFunction("int get_jjWaterLayer()", asFUNCTION(get_waterLayer), asCALL_CDECL);
		engine->RegisterGlobalFunction("int set_jjWaterLayer(int)", asFUNCTION(set_waterLayer), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjSetWaterGradient(uint8 red1, uint8 green1, uint8 blue1, uint8 red2, uint8 green2, uint8 blue2)", asFUNCTION(setWaterGradient), asCALL_CDECL);
		// TODO
		//engine->RegisterGlobalFunction("void jjSetWaterGradient(jjPALCOLOR color1, jjPALCOLOR color2)", asFUNCTION(setWaterGradientFromColors), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjSetWaterGradient()", asFUNCTION(setWaterGradientToTBG), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjResetWaterGradient()", asFUNCTION(resetWaterGradient), asCALL_CDECL);

		engine->RegisterGlobalFunction("void jjTriggerRock(uint8 id)", asFUNCTION(triggerRock), asCALL_CDECL);

		engine->RegisterGlobalFunction("void jjNxt(const string &in filename, bool warp = false, bool fast = false)", asFUNCTION(cycleTo), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjNxt(bool warp = false, bool fast = false)", asFUNCTION(jjNxt), asCALL_CDECL);

		engine->RegisterGlobalFunction("bool get_jjEnabledTeams(uint8)", asFUNCTION(getEnabledTeam), asCALL_CDECL);

		engine->RegisterGlobalProperty("uint8 jjKeyChat", &ChatKey);
		engine->RegisterGlobalFunction("bool get_jjKey(uint8)", asFUNCTION(getKeyDown), asCALL_CDECL);
		engine->RegisterGlobalFunction("int get_jjMouseX()", asFUNCTION(getCursorX), asCALL_CDECL);
		engine->RegisterGlobalFunction("int get_jjMouseY()", asFUNCTION(getCursorY), asCALL_CDECL);

		engine->RegisterGlobalFunction("bool jjMusicLoad(string &in filename, bool forceReload = false, bool temporary = false)", asFUNCTION(jjMusicLoad), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjMusicStop()", asFUNCTION(jjMusicStop), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjMusicPlay()", asFUNCTION(jjMusicPlay), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjMusicPause()", asFUNCTION(jjMusicPause), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjMusicResume()", asFUNCTION(jjMusicResume), asCALL_CDECL);

		engine->SetDefaultNamespace("SOUND");
		engine->RegisterEnum("Sample");
		engine->SetDefaultNamespace("");
		engine->RegisterGlobalFunction("void jjSample(float xPixel, float yPixel, SOUND::Sample sample, int volume = 63, int frequency = 0)", asFUNCTION(playSample), asCALL_CDECL);
		engine->RegisterGlobalFunction("int jjSampleLooped(float xPixel, float yPixel, SOUND::Sample sample, int channel, int volume = 63, int frequency = 0)", asFUNCTION(playLoopedSample), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjSamplePriority(SOUND::Sample sample)", asFUNCTION(playPrioritySample), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool jjSampleIsLoaded(SOUND::Sample sample)", asFUNCTION(isSampleLoaded), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool jjSampleLoad(SOUND::Sample sample, string& in filename)", asFUNCTION(loadSample), asCALL_CDECL);

		engine->RegisterGlobalProperty("const bool jjSoundEnabled", &soundEnabled);
		engine->RegisterGlobalProperty("const bool jjSoundFXActive", &soundFXActive);
		engine->RegisterGlobalProperty("const bool jjMusicActive", &musicActive);
		engine->RegisterGlobalProperty("const int jjSoundFXVolume", &soundFXVolume);
		engine->RegisterGlobalProperty("const int jjMusicVolume", &musicVolume);
		engine->RegisterGlobalProperty("int jjEcho", &levelEcho);

		engine->RegisterGlobalProperty("bool jjWarpsTransmuteCoins", &warpsTransmuteCoins);
		engine->RegisterGlobalProperty("bool jjDelayGeneratedCrateOrigins", &delayGeneratedCrateOrigins);
		engine->RegisterGlobalFunction("bool get_jjUseLayer8Speeds()", asFUNCTION(getUseLayer8Speeds), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool set_jjUseLayer8Speeds(bool)", asFUNCTION(setUseLayer8Speeds), asCALL_CDECL);

		engine->RegisterGlobalProperty("bool jjSugarRushAllowed", &g_levelHasFood);
		engine->RegisterGlobalProperty("bool jjSugarRushesAllowed", &g_levelHasFood);

		engine->RegisterObjectType("jjWEAPON", sizeof(jjWEAPON), asOBJ_REF | asOBJ_NOCOUNT);
		// TODO
		/*engine->RegisterGlobalFunction("jjWEAPON@ get_jjWeapons(int)", asFUNCTION(gejjWEAPON), asCALL_CDECL);
		engine->RegisterObjectProperty("jjWEAPON", "bool infinite", asOFFSET(jjWEAPON, infinite));
		engine->RegisterObjectProperty("jjWEAPON", "bool replenishes", asOFFSET(jjWEAPON, replenishes));
		engine->RegisterObjectProperty("jjWEAPON", "bool replacedByShield", asOFFSET(jjWEAPON, shield));
		engine->RegisterObjectProperty("jjWEAPON", "bool replacedByBubbles", asOFFSET(jjWEAPON, bubbles));
		engine->RegisterObjectProperty("jjWEAPON", "bool comesFromGunCrates", asOFFSET(jjWEAPON, crates));
		engine->RegisterObjectProperty("jjWEAPON", "bool gradualAim", asOFFSET(jjWEAPON, gradual));
		engine->RegisterObjectProperty("jjWEAPON", "int multiplier", asOFFSET(jjWEAPON, multiplier));
		engine->RegisterObjectProperty("jjWEAPON", "int maximum", asOFFSET(jjWEAPON, maximum));
		engine->RegisterObjectProperty("jjWEAPON", "int gemsLost", asOFFSET(jjWEAPON, gemsLost));
		engine->RegisterObjectProperty("jjWEAPON", "int gemsLostPowerup", asOFFSET(jjWEAPON, gemsLostPowerup));
		engine->RegisterObjectProperty("jjWEAPON", "int8 style", asOFFSET(jjWEAPON, style));
		engine->RegisterObjectProperty("jjWEAPON", "SPREAD::Spread spread", asOFFSET(jjWEAPON, spread));
		engine->RegisterObjectProperty("jjWEAPON", "bool defaultSample", asOFFSET(jjWEAPON, sample));
		engine->RegisterObjectProperty("jjWEAPON", "bool allowed", asOFFSET(jjWEAPON, appearsInLevel));
		engine->RegisterObjectProperty("jjWEAPON", "bool allowedPowerup", asOFFSET(jjWEAPON, powerupAppearsInLevel));
		engine->RegisterObjectProperty("jjWEAPON", "bool comesFromBirds", asOFFSET(jjWEAPON, canBeShotByBirds));
		engine->RegisterObjectProperty("jjWEAPON", "bool comesFromBirdsPowerup", asOFFSET(jjWEAPON, powerupCanBeShotByBirds));*/

		engine->SetDefaultNamespace("AIR");
		engine->RegisterEnum("Jump");
		engine->RegisterEnumValue("Jump", "NONE", airjumpNONE);
		engine->RegisterEnumValue("Jump", "HELICOPTER", airjumpHELICOPTER);
		engine->RegisterEnumValue("Jump", "DOUBLEJUMP", airjumpSPAZ);
		engine->SetDefaultNamespace("GROUND");
		engine->RegisterEnum("Jump");
		engine->RegisterEnumValue("Jump", "CROUCH", groundjumpNONE);
		engine->RegisterEnumValue("Jump", "JUMP", groundjumpREGULARJUMP);
		engine->RegisterEnumValue("Jump", "JAZZ", groundjumpJAZZ);
		engine->RegisterEnumValue("Jump", "SPAZ", groundjumpSPAZ);
		engine->RegisterEnumValue("Jump", "LORI", groundjumpLORI);
		engine->SetDefaultNamespace("");
		engine->RegisterObjectType("jjCHARACTER", sizeof(jjCHARACTER), asOBJ_REF | asOBJ_NOCOUNT);
		/*engine->RegisterGlobalFunction("jjCHARACTER@ get_jjCharacters(CHAR::Char)", asFUNCTION(gejjCHARACTER), asCALL_CDECL);
		engine->RegisterObjectProperty("jjCHARACTER", "AIR::Jump airJump", asOFFSET(jjCHARACTER, airJump));
		engine->RegisterObjectProperty("jjCHARACTER", "GROUND::Jump groundJump", asOFFSET(jjCHARACTER, groundJump));
		engine->RegisterObjectProperty("jjCHARACTER", "int doubleJumpCountMax", asOFFSET(jjCHARACTER, doubleJumpCountMax));
		REGISTER_FLOAT_PROPERTY("jjCHARACTER", "doubleJumpXSpeed", jjCHARACTER, doubleJumpXSpeed);
		REGISTER_FLOAT_PROPERTY("jjCHARACTER", "doubleJumpYSpeed", jjCHARACTER, doubleJumpYSpeed);
		engine->RegisterObjectProperty("jjCHARACTER", "int helicopterDurationMax", asOFFSET(jjCHARACTER, helicopterDurationMax));
		REGISTER_FLOAT_PROPERTY("jjCHARACTER", "helicopterXSpeed", jjCHARACTER, helicopterXSpeed);
		REGISTER_FLOAT_PROPERTY("jjCHARACTER", "helicopterYSpeed", jjCHARACTER, helicopterYSpeed);
		engine->RegisterObjectProperty("jjCHARACTER", "bool canHurt", asOFFSET(jjCHARACTER, specialMovesDoDamage));
		engine->RegisterObjectProperty("jjCHARACTER", "bool canRun", asOFFSET(jjCHARACTER, canRun));
		engine->RegisterObjectProperty("jjCHARACTER", "bool morphBoxCycle", asOFFSET(jjCHARACTER, availableToMorphBoxes));*/

		engine->SetDefaultNamespace("CREATOR");
		engine->RegisterEnum("Type");
		engine->RegisterEnumValue("Type", "OBJECT", 0);
		engine->RegisterEnumValue("Type", "LEVEL", 1);
		engine->RegisterEnumValue("Type", "PLAYER", 2);
		engine->SetDefaultNamespace("AREA");
		engine->RegisterEnum("Area");
		engine->SetDefaultNamespace("OBJECT");
		engine->RegisterEnum("Object");
		engine->SetDefaultNamespace("ANIM");
		engine->RegisterEnum("Set");
		engine->SetDefaultNamespace("");

		engine->RegisterGlobalFunction("int jjEventGet(uint16 xTile, uint16 yTile)", asFUNCTION(GetEvent), asCALL_CDECL);
		engine->RegisterGlobalFunction("int jjParameterGet(uint16 xTile, uint16 yTile, int offset, int length)", asFUNCTION(GetEventParamWrapper), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjEventSet(uint16 xTile, uint16 yTile, uint8 newEventID)", asFUNCTION(SetEventByte), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjParameterSet(uint16 xTile, uint16 yTile, int8 offset, int8 length, int newValue)", asFUNCTION(SetEventParam), asCALL_CDECL);
		engine->RegisterGlobalFunction("uint8 get_jjTileType(uint16)", asFUNCTION(GetTileType), asCALL_CDECL);
		engine->RegisterGlobalFunction("uint8 set_jjTileType(uint16,uint8)", asFUNCTION(SetTileType), asCALL_CDECL);

		engine->SetDefaultNamespace("LIGHT");
		engine->RegisterEnum("Enforce");
		engine->RegisterEnumValue("Enforce", "OPTIONAL", ambientLighting_OPTIONAL);
		engine->RegisterEnumValue("Enforce", "BASIC", ambientLighting_BASIC);
		engine->RegisterEnumValue("Enforce", "COMPLETE", ambientLighting_COMPLETE);

		engine->SetDefaultNamespace("");
		engine->RegisterGlobalProperty("LIGHT::Enforce jjEnforceLighting", &enforceAmbientLighting);

		engine->SetDefaultNamespace("STATE");
		engine->RegisterEnum("State");
		engine->SetDefaultNamespace("BEHAVIOR");
		engine->RegisterEnum("Behavior");

		engine->SetDefaultNamespace("LIGHT");
		engine->RegisterEnum("Type");
		engine->RegisterEnumValue("Type", "NONE", 0);
		engine->RegisterEnumValue("Type", "NORMAL", 3);
		engine->RegisterEnumValue("Type", "POINT", 1);
		engine->RegisterEnumValue("Type", "POINT2", 2);
		engine->RegisterEnumValue("Type", "FLICKER", 4);
		engine->RegisterEnumValue("Type", "BRIGHT", 5);
		engine->RegisterEnumValue("Type", "LASERBEAM", 6);
		engine->RegisterEnumValue("Type", "LASER", 7);
		engine->RegisterEnumValue("Type", "RING", 8);
		engine->RegisterEnumValue("Type", "RING2", 9);
		engine->RegisterEnumValue("Type", "PLAYER", 10);

		engine->SetDefaultNamespace("HANDLING");
		engine->RegisterEnum("Bullet");
		engine->RegisterEnumValue("Bullet", "HURTBYBULLET", 0);
		engine->RegisterEnumValue("Bullet", "IGNOREBULLET", 1);
		engine->RegisterEnumValue("Bullet", "DESTROYBULLET", 2);
		engine->RegisterEnumValue("Bullet", "DETECTBULLET", 3);
		engine->RegisterEnum("Player");
		engine->RegisterEnumValue("Player", "ENEMY", 0);
		engine->RegisterEnumValue("Player", "PLAYERBULLET", 1);
		engine->RegisterEnumValue("Player", "ENEMYBULLET", 2);
		engine->RegisterEnumValue("Player", "PARTICLE", 3);
		engine->RegisterEnumValue("Player", "EXPLOSION", 4);
		engine->RegisterEnumValue("Player", "PICKUP", 5);
		engine->RegisterEnumValue("Player", "DELAYEDPICKUP", 6);
		engine->RegisterEnumValue("Player", "HURT", 7);
		engine->RegisterEnumValue("Player", "SPECIAL", 8);
		engine->RegisterEnumValue("Player", "DYING", 9);
		engine->RegisterEnumValue("Player", "SPECIALDONE", 10);
		engine->RegisterEnumValue("Player", "SELFCOLLISION", 11);
		engine->SetDefaultNamespace("");

		// TODO
		engine->RegisterObjectType("jjOBJ", sizeof(jjOBJ), asOBJ_REF /*| asOBJ_NOCOUNT*/);
		engine->RegisterObjectBehaviour("jjOBJ", asBEHAVE_ADDREF, "void f()", asMETHOD(jjOBJ, AddRef), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("jjOBJ", asBEHAVE_RELEASE, "void f()", asMETHOD(jjOBJ, Release), asCALL_THISCALL);
		engine->RegisterGlobalFunction("jjOBJ @get_jjObjects(int)", asFUNCTION(get_jjObjects), asCALL_CDECL);
		engine->RegisterGlobalFunction("jjOBJ @get_jjObjectPresets(uint8)", asFUNCTION(get_jjObjectPresets), asCALL_CDECL);
		engine->RegisterGlobalProperty("const int jjObjectCount", &jjObjectCount);
		engine->RegisterGlobalProperty("const int jjObjectMax", &jjObjectMax);
		engine->RegisterObjectMethod("jjOBJ", "bool get_isActive() const", asMETHOD(jjOBJ, get_isActive), asCALL_THISCALL);

		engine->RegisterObjectMethod("jjPLAYER", "LIGHT::Type get_lightType() const", asMETHOD(jjOBJ, get_lightType), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "LIGHT::Type set_lightType(LIGHT::Type)", asMETHOD(jjOBJ, set_lightType), asCALL_THISCALL);

		engine->RegisterObjectMethod("jjPLAYER", "bool doesCollide(const jjOBJ@ object, bool always = false) const", asMETHOD(jjPLAYER, doesCollide), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "int getObjectHitForce(const jjOBJ@ target = null) const", asMETHOD(jjPLAYER, getObjectHitForce), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYER", "bool objectHit(jjOBJ@ target, int force, HANDLING::Player playerHandling)", asMETHOD(jjPLAYER, objectHit), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "void objectHit(jjOBJ@ target, HANDLING::Player playerHandling)", asMETHOD(jjOBJ, objectHit), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "void blast(int maxDistance, bool blastObjects)", asMETHOD(jjOBJ, blast), asCALL_THISCALL);

		engine->RegisterObjectMethod("jjPLAYER", "bool isEnemy(const jjPLAYER &in victim) const", asMETHOD(jjPLAYER, isEnemy), asCALL_THISCALL);

		engine->RegisterObjectProperty("jjPLAYER", "const ANIM::Set setID", asOFFSET(jjPLAYER, charCurr));
		engine->RegisterObjectProperty("jjPLAYER", "const uint16 curAnim", asOFFSET(jjPLAYER, curAnim));
		engine->RegisterObjectProperty("jjPLAYER", "const uint curFrame", asOFFSET(jjPLAYER, curFrame));
		engine->RegisterObjectProperty("jjPLAYER", "const uint8 frameID", asOFFSET(jjPLAYER, frameID));

		engine->RegisterFuncdef("void jjVOIDFUNCOBJ(jjOBJ@)");
		engine->RegisterObjectType("jjBEHAVIOR", sizeof(jjBEHAVIOR), asOBJ_VALUE | asOBJ_APP_CLASS_CDA);
		engine->RegisterObjectBehaviour("jjBEHAVIOR", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(jjBEHAVIOR::Create), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("jjBEHAVIOR", asBEHAVE_CONSTRUCT, "void f(const BEHAVIOR::Behavior &in behavior)", asFUNCTION(jjBEHAVIOR::CreateFromBehavior), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("jjBEHAVIOR", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(jjBEHAVIOR::Destroy), asCALL_CDECL_OBJLAST);

		engine->RegisterInterface("jjBEHAVIORINTERFACE");
		engine->RegisterInterfaceMethod("jjBEHAVIORINTERFACE", "void onBehave(jjOBJ@ obj)");

		engine->RegisterObjectMethod("jjBEHAVIOR", "jjBEHAVIOR& opAssign(const jjBEHAVIOR &in)", asMETHODPR(jjBEHAVIOR, operator=, (const jjBEHAVIOR&), jjBEHAVIOR&), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjBEHAVIOR", "jjBEHAVIOR& opAssign(BEHAVIOR::Behavior)", asMETHODPR(jjBEHAVIOR, operator=, (uint32_t), jjBEHAVIOR&), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjBEHAVIOR", "jjBEHAVIOR& opAssign(jjVOIDFUNCOBJ@)", asMETHODPR(jjBEHAVIOR, operator=, (asIScriptFunction*), jjBEHAVIOR&), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjBEHAVIOR", "jjBEHAVIOR& opAssign(jjBEHAVIORINTERFACE@)", asMETHODPR(jjBEHAVIOR, operator=, (asIScriptObject*), jjBEHAVIOR&), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjBEHAVIOR", "bool opEquals(const jjBEHAVIOR &in) const", asMETHODPR(jjBEHAVIOR, operator==, (const jjBEHAVIOR&) const, bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjBEHAVIOR", "bool opEquals(BEHAVIOR::Behavior) const", asMETHODPR(jjBEHAVIOR, operator==, (uint32_t) const, bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjBEHAVIOR", "bool opEquals(const jjVOIDFUNCOBJ@) const", asMETHODPR(jjBEHAVIOR, operator==, (const asIScriptFunction*) const, bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjBEHAVIOR", "BEHAVIOR::Behavior opConv() const", asMETHOD(jjBEHAVIOR, operator uint32_t), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjBEHAVIOR", "jjVOIDFUNCOBJ@ opCast() const", asMETHOD(jjBEHAVIOR, operator asIScriptFunction*), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjBEHAVIOR", "jjBEHAVIORINTERFACE@ opCast() const", asMETHOD(jjBEHAVIOR, operator asIScriptObject*), asCALL_THISCALL);

		engine->RegisterObjectProperty("jjOBJ", "jjBEHAVIOR behavior", asOFFSET(jjOBJ, behavior));

		engine->RegisterObjectMethod("jjOBJ", "void behave(BEHAVIOR::Behavior behavior = BEHAVIOR::DEFAULT, bool draw = true)", asMETHOD(jjOBJ, behave1), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "void behave(jjBEHAVIOR behavior, bool draw = true)", asMETHOD(jjOBJ, behave2), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "void behave(jjVOIDFUNCOBJ@ behavior, bool draw = true)", asMETHOD(jjOBJ, behave3), asCALL_THISCALL);

		engine->RegisterGlobalFunction("int jjAddObject(uint8 eventID, float xPixel, float yPixel, uint16 creatorID = 0, CREATOR::Type creatorType = CREATOR::OBJECT, BEHAVIOR::Behavior behavior = BEHAVIOR::DEFAULT)", asFUNCTION(jjOBJ::jjAddObject), asCALL_CDECL);
		engine->RegisterGlobalFunction("int jjAddObject(uint8 eventID, float xPixel, float xPixel, uint16 creatorID, CREATOR::Type creatorType, jjVOIDFUNCOBJ@ behavior)", asFUNCTION(jjOBJ::jjAddObjectEx), asCALL_CDECL);

		engine->RegisterObjectProperty("jjOBJ", "float xOrg", asOFFSET(jjOBJ, xOrg));
		engine->RegisterObjectProperty("jjOBJ", "float yOrg", asOFFSET(jjOBJ, yOrg));
		engine->RegisterObjectProperty("jjOBJ", "float xPos", asOFFSET(jjOBJ, xPos));
		engine->RegisterObjectProperty("jjOBJ", "float yPos", asOFFSET(jjOBJ, yPos));
		engine->RegisterObjectProperty("jjOBJ", "float xSpeed", asOFFSET(jjOBJ, xSpeed));
		engine->RegisterObjectProperty("jjOBJ", "float ySpeed", asOFFSET(jjOBJ, ySpeed));
		engine->RegisterObjectProperty("jjOBJ", "float xAcc", asOFFSET(jjOBJ, xAcc));
		engine->RegisterObjectProperty("jjOBJ", "float yAcc", asOFFSET(jjOBJ, yAcc));
		engine->RegisterObjectProperty("jjOBJ", "int counter", asOFFSET(jjOBJ, counter));
		engine->RegisterObjectProperty("jjOBJ", "uint curFrame", asOFFSET(jjOBJ, curFrame));
		engine->RegisterObjectMethod("jjOBJ", "uint determineCurFrame(bool change = true)", asMETHOD(jjOBJ, determineCurFrame), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjOBJ", "int age", asOFFSET(jjOBJ, age));
		engine->RegisterObjectProperty("jjOBJ", "int creator", asOFFSET(jjOBJ, creator)); // Deprecated
		engine->RegisterObjectMethod("jjOBJ", "uint16 get_creatorID() const", asMETHOD(jjOBJ, get_creatorID), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "uint16 set_creatorID(uint16)", asMETHOD(jjOBJ, set_creatorID), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "CREATOR::Type get_creatorType() const", asMETHOD(jjOBJ, get_creatorType), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "CREATOR::Type set_creatorType(CREATOR::Type)", asMETHOD(jjOBJ, set_creatorType), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjOBJ", "int16 curAnim", asOFFSET(jjOBJ, curAnim));
		engine->RegisterObjectMethod("jjOBJ", "int16 determineCurAnim(uint8 setID, uint8 animation, bool change = true)", asMETHOD(jjOBJ, determineCurAnim), asCALL_THISCALL);

		engine->RegisterObjectProperty("jjOBJ", "int16 killAnim", asOFFSET(jjOBJ, killAnim));
		engine->RegisterObjectProperty("jjOBJ", "uint8 freeze", asOFFSET(jjOBJ, freeze));
		engine->RegisterObjectProperty("jjOBJ", "uint8 lightType", asOFFSET(jjOBJ, lightType));
		engine->RegisterObjectProperty("jjOBJ", "int8 frameID", asOFFSET(jjOBJ, frameID));
		engine->RegisterObjectProperty("jjOBJ", "int8 noHit", asOFFSET(jjOBJ, noHit)); // Deprecated
		engine->RegisterObjectMethod("jjOBJ", "HANDLING::Bullet get_bulletHandling() const", asMETHOD(jjOBJ, get_bulletHandling), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "HANDLING::Bullet set_bulletHandling(HANDLING::Bullet)", asMETHOD(jjOBJ, set_bulletHandling), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "bool get_causesRicochet() const", asMETHOD(jjOBJ, get_ricochet), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "bool set_causesRicochet(bool)", asMETHOD(jjOBJ, set_ricochet), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "bool get_isFreezable() const", asMETHOD(jjOBJ, get_freezable), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "bool set_isFreezable(bool)", asMETHOD(jjOBJ, set_freezable), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "bool get_isBlastable() const", asMETHOD(jjOBJ, get_blastable), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "bool set_isBlastable(bool)", asMETHOD(jjOBJ, set_blastable), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjOBJ", "int8 energy", asOFFSET(jjOBJ, energy));
		engine->RegisterObjectProperty("jjOBJ", "int8 light", asOFFSET(jjOBJ, light));
		engine->RegisterObjectProperty("jjOBJ", "uint8 objType", asOFFSET(jjOBJ, objType)); // Deprecated
		engine->RegisterObjectMethod("jjOBJ", "HANDLING::Player get_playerHandling() const", asMETHOD(jjOBJ, get_playerHandling), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "HANDLING::Player set_playerHandling(HANDLING::Player)", asMETHOD(jjOBJ, set_playerHandling), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "bool get_isTarget() const", asMETHOD(jjOBJ, get_isTarget), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "bool set_isTarget(bool)", asMETHOD(jjOBJ, set_isTarget), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "bool get_triggersTNT() const", asMETHOD(jjOBJ, get_triggersTNT), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "bool set_triggersTNT(bool)", asMETHOD(jjOBJ, set_triggersTNT), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "bool get_deactivates() const", asMETHOD(jjOBJ, get_deactivates), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "bool set_deactivates(bool)", asMETHOD(jjOBJ, set_deactivates), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "bool get_scriptedCollisions() const", asMETHOD(jjOBJ, get_scriptedCollisions), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "bool set_scriptedCollisions(bool)", asMETHOD(jjOBJ, set_scriptedCollisions), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjOBJ", "int8 state", asOFFSET(jjOBJ, state));
		engine->RegisterObjectProperty("jjOBJ", "uint16 points", asOFFSET(jjOBJ, points));
		engine->RegisterObjectProperty("jjOBJ", "uint8 eventID", asOFFSET(jjOBJ, eventID));
		engine->RegisterObjectProperty("jjOBJ", "int8 direction", asOFFSET(jjOBJ, direction));
		engine->RegisterObjectProperty("jjOBJ", "uint8 justHit", asOFFSET(jjOBJ, justHit));
		engine->RegisterObjectProperty("jjOBJ", "int8 oldState", asOFFSET(jjOBJ, oldState));
		engine->RegisterObjectProperty("jjOBJ", "int animSpeed", asOFFSET(jjOBJ, animSpeed));
		engine->RegisterObjectProperty("jjOBJ", "int special", asOFFSET(jjOBJ, special));
		engine->RegisterObjectMethod("jjOBJ", "int get_var(uint8) const", asMETHOD(jjOBJ, get_var), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "int set_var(uint8, int)", asMETHOD(jjOBJ, set_var), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjOBJ", "uint8 doesHurt", asOFFSET(jjOBJ, doesHurt));
		engine->RegisterObjectProperty("jjOBJ", "uint8 counterEnd", asOFFSET(jjOBJ, counterEnd));
		engine->RegisterObjectProperty("jjOBJ", "const int16 objectID", asOFFSET(jjOBJ, objectID));

		engine->RegisterGlobalFunction("void jjDeleteObject(int objectID)", asFUNCTION(jjOBJ::jjDeleteObject), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjKillObject(int objectID)", asFUNCTION(jjOBJ::jjKillObject), asCALL_CDECL);
		engine->RegisterGlobalProperty("const bool jjDeactivatingBecauseOfDeath", &jjDeactivatingBecauseOfDeath);

		engine->RegisterObjectMethod("jjOBJ", "int draw()", asMETHOD(jjOBJ, draw), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "int beSolid(bool shouldCheckForStompingLocalPlayers = false)", asMETHOD(jjOBJ, beSolid), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "void bePlatform(float xOld, float yOld, int width = 0, int height = 0)", asMETHOD(jjOBJ, bePlatform), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "void clearPlatform()", asMETHOD(jjOBJ, clearPlatform), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "void putOnGround(bool precise = false)", asMETHOD(jjOBJ, putOnGround), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "bool ricochet()", asMETHOD(jjOBJ, ricochet), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "int unfreeze(int style)", asMETHOD(jjOBJ, unfreeze), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "void delete()", asMETHOD(jjOBJ, deleteObject), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "void deactivate()", asMETHOD(jjOBJ, deactivate), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "void pathMovement()", asMETHOD(jjOBJ, pathMovement), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "int fireBullet(uint8 eventID) const", asMETHOD(jjOBJ, fireBullet), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "void particlePixelExplosion(int style) const", asMETHOD(jjOBJ, particlePixelExplosion), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "void grantPickup(jjPLAYER@ player, int frequency) const", asMETHOD(jjOBJ, grantPickup), asCALL_THISCALL);

		engine->RegisterObjectMethod("jjOBJ", "int findNearestPlayer(int maxDistance) const", asMETHOD(jjOBJ, findNearestPlayer), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "int findNearestPlayer(int maxDistance, int &out foundDistance) const", asMETHOD(jjOBJ, findNearestPlayerEx), asCALL_THISCALL);

		engine->RegisterObjectMethod("jjOBJ", "bool doesCollide(const jjOBJ@ object, bool always = false) const", asMETHOD(jjOBJ, doesCollide), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjOBJ", "bool doesCollide(const jjPLAYER@ player, bool always = false) const", asMETHOD(jjOBJ, doesCollidePlayer), asCALL_THISCALL);

		// TODO
		/*engine->RegisterGlobalFunction("void jjAddParticleTileExplosion(uint16 xTile, uint16 yTile, uint16 tile, bool collapseSceneryStyle)", asFUNCTION(ExternalAddParticleTile), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjAddParticlePixelExplosion(float xPixel, float yPixel, int curFrame, int direction, int mode)", asFUNCTION(addParticlePixelExplosion), asCALL_CDECL);*/

		engine->SetDefaultNamespace("PARTICLE");
		engine->RegisterEnum("Type");
		engine->RegisterEnumValue("Type", "INACTIVE", particleNONE);
		engine->RegisterEnumValue("Type", "PIXEL", particlePIXEL);
		engine->RegisterEnumValue("Type", "FIRE", particleFIRE);
		engine->RegisterEnumValue("Type", "SMOKE", particleSMOKE);
		engine->RegisterEnumValue("Type", "ICETRAIL", particleICETRAIL);
		engine->RegisterEnumValue("Type", "SPARK", particleSPARK);
		engine->RegisterEnumValue("Type", "STRING", particleSCORE);
		engine->RegisterEnumValue("Type", "SNOW", particleSNOW);
		engine->RegisterEnumValue("Type", "RAIN", particleRAIN);
		engine->RegisterEnumValue("Type", "FLOWER", particleFLOWER);
		engine->RegisterEnumValue("Type", "LEAF", particleLEAF);
		engine->RegisterEnumValue("Type", "STAR", particleSTAR);
		engine->RegisterEnumValue("Type", "TILE", particleTILE);
		engine->SetDefaultNamespace("");

		// TODO
		/*engine->RegisterObjectType("jjPARTICLE", sizeof(Tparticle), asOBJ_REF | asOBJ_NOCOUNT);
		engine->RegisterGlobalFunction("jjPARTICLE @get_jjParticles(int)", asFUNCTION(getParticle), asCALL_CDECL);
		engine->RegisterGlobalFunction("jjPARTICLE @jjAddParticle(PARTICLE::Type type)", asFUNCTION(AddParticle), asCALL_CDECL);
		REGISTER_FLOAT_PROPERTY("jjPARTICLE", "xPos", Tparticle, xPos);
		REGISTER_FLOAT_PROPERTY("jjPARTICLE", "yPos", Tparticle, yPos);
		REGISTER_FLOAT_PROPERTY("jjPARTICLE", "xSpeed", Tparticle, xSpeed);
		REGISTER_FLOAT_PROPERTY("jjPARTICLE", "ySpeed", Tparticle, ySpeed);
		engine->RegisterObjectProperty("jjPARTICLE", "uint8 type", asOFFSET(Tparticle, particleType));
		engine->RegisterObjectProperty("jjPARTICLE", "bool isActive", asOFFSET(Tparticle, active));
		engine->RegisterObjectType("jjPARTICLEPIXEL", 9, asOBJ_VALUE | asOBJ_POD); // Private/deprecated
		engine->RegisterObjectProperty("jjPARTICLEPIXEL", "uint8 size", -2);
		engine->RegisterObjectMethod("jjPARTICLEPIXEL", "uint8 get_color(int) const", asMETHOD(TparticlePIXEL, get_color), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPARTICLEPIXEL", "uint8 set_color(int, uint8)", asMETHOD(TparticlePIXEL, set_color), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjPARTICLE", "jjPARTICLEPIXEL pixel", asOFFSET(Tparticle, GENERIC));
		engine->RegisterObjectType("jjPARTICLEFIRE", 9, asOBJ_VALUE | asOBJ_POD);  // Private/deprecated
		engine->RegisterObjectProperty("jjPARTICLEFIRE", "uint8 size", -2);
		engine->RegisterObjectProperty("jjPARTICLEFIRE", "uint8 color", 0);
		engine->RegisterObjectProperty("jjPARTICLEFIRE", "uint8 colorStop", 1);
		engine->RegisterObjectProperty("jjPARTICLEFIRE", "int8 colorDelta", 2);
		engine->RegisterObjectProperty("jjPARTICLE", "jjPARTICLEFIRE fire", asOFFSET(Tparticle, GENERIC));
		engine->RegisterObjectType("jjPARTICLESMOKE", 9, asOBJ_VALUE | asOBJ_POD);  // Private/deprecated
		engine->RegisterObjectProperty("jjPARTICLESMOKE", "uint8 countdown", 0);
		engine->RegisterObjectProperty("jjPARTICLE", "jjPARTICLESMOKE smoke", asOFFSET(Tparticle, GENERIC));
		engine->RegisterObjectType("jjPARTICLEICETRAIL", 9, asOBJ_VALUE | asOBJ_POD);
		engine->RegisterObjectProperty("jjPARTICLEICETRAIL", "uint8 color", 0);
		engine->RegisterObjectProperty("jjPARTICLEICETRAIL", "uint8 colorStop", 1);
		engine->RegisterObjectProperty("jjPARTICLEICETRAIL", "int8 colorDelta", 2);
		engine->RegisterObjectProperty("jjPARTICLE", "jjPARTICLEICETRAIL icetrail", asOFFSET(Tparticle, GENERIC));
		engine->RegisterObjectType("jjPARTICLESPARK", 9, asOBJ_VALUE | asOBJ_POD);  // Private/deprecated
		engine->RegisterObjectProperty("jjPARTICLESPARK", "uint8 color", 0);
		engine->RegisterObjectProperty("jjPARTICLESPARK", "uint8 colorStop", 1);
		engine->RegisterObjectProperty("jjPARTICLESPARK", "int8 colorDelta", 2);
		engine->RegisterObjectProperty("jjPARTICLE", "jjPARTICLESPARK spark", asOFFSET(Tparticle, GENERIC));
		engine->RegisterObjectType("jjPARTICLESTRING", 9, asOBJ_VALUE | asOBJ_POD); // Private/deprecated
		engine->RegisterObjectMethod("jjPARTICLESTRING", "::string get_text() const", asMETHOD(TparticleSCORE, get_text), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPARTICLESTRING", "void set_text(::string)", asMETHOD(TparticleSCORE, set_text), asCALL_THISCALL);

		engine->RegisterObjectProperty("jjPARTICLE", "jjPARTICLESTRING string", asOFFSET(Tparticle, GENERIC));
		engine->RegisterObjectType("jjPARTICLESNOW", 9, asOBJ_VALUE | asOBJ_POD); // Private/deprecated
		engine->RegisterObjectProperty("jjPARTICLESNOW", "uint8 frame", 0);
		engine->RegisterObjectProperty("jjPARTICLESNOW", "uint8 countup", 1);
		engine->RegisterObjectProperty("jjPARTICLESNOW", "uint8 countdown", 2);
		engine->RegisterObjectProperty("jjPARTICLESNOW", "uint16 frameBase", -7);
		engine->RegisterObjectProperty("jjPARTICLE", "jjPARTICLESNOW snow", asOFFSET(Tparticle, GENERIC));
		engine->RegisterObjectType("jjPARTICLERAIN", 9, asOBJ_VALUE | asOBJ_POD); // Private/deprecated
		engine->RegisterObjectProperty("jjPARTICLERAIN", "uint8 frame", 0);
		engine->RegisterObjectProperty("jjPARTICLERAIN", "uint16 frameBase", -7);
		engine->RegisterObjectProperty("jjPARTICLE", "jjPARTICLERAIN rain", asOFFSET(Tparticle, GENERIC));
		engine->RegisterObjectType("jjPARTICLELEAF", 9, asOBJ_VALUE | asOBJ_POD); // Private/deprecated
		engine->RegisterObjectProperty("jjPARTICLELEAF", "uint8 frame", 0);
		engine->RegisterObjectProperty("jjPARTICLELEAF", "uint8 countup", 1);
		engine->RegisterObjectProperty("jjPARTICLELEAF", "bool noclip", 2);
		engine->RegisterObjectProperty("jjPARTICLELEAF", "uint8 height", 3);
		engine->RegisterObjectProperty("jjPARTICLELEAF", "uint16 frameBase", -7);
		engine->RegisterObjectProperty("jjPARTICLE", "jjPARTICLELEAF leaf", asOFFSET(Tparticle, GENERIC));
		engine->RegisterObjectType("jjPARTICLEFLOWER", 9, asOBJ_VALUE | asOBJ_POD); // Private/deprecated
		engine->RegisterObjectProperty("jjPARTICLEFLOWER", "uint8 size", -2);
		engine->RegisterObjectProperty("jjPARTICLEFLOWER", "uint8 color", 0);
		engine->RegisterObjectProperty("jjPARTICLEFLOWER", "uint8 angle", 1);
		engine->RegisterObjectProperty("jjPARTICLEFLOWER", "int8 angularSpeed", 2);
		engine->RegisterObjectProperty("jjPARTICLEFLOWER", "uint8 petals", 3);
		engine->RegisterObjectProperty("jjPARTICLE", "jjPARTICLEFLOWER flower", asOFFSET(Tparticle, GENERIC));
		engine->RegisterObjectType("jjPARTICLESTAR", 9, asOBJ_VALUE | asOBJ_POD); // Private/deprecated
		engine->RegisterObjectProperty("jjPARTICLESTAR", "uint8 size", -2);
		engine->RegisterObjectProperty("jjPARTICLESTAR", "uint8 color", 0);
		engine->RegisterObjectProperty("jjPARTICLESTAR", "uint8 angle", 1);
		engine->RegisterObjectProperty("jjPARTICLESTAR", "int8 angularSpeed", 2);
		engine->RegisterObjectProperty("jjPARTICLESTAR", "uint8 frame", 3);
		engine->RegisterObjectProperty("jjPARTICLESTAR", "uint8 colorChangeCounter", 4);
		engine->RegisterObjectProperty("jjPARTICLESTAR", "uint8 colorChangeInterval", 5);
		engine->RegisterObjectProperty("jjPARTICLE", "jjPARTICLESTAR star", asOFFSET(Tparticle, GENERIC));
		engine->RegisterObjectType("jjPARTICLETILE", 9, asOBJ_VALUE | asOBJ_POD); // Private/deprecated
		engine->RegisterObjectProperty("jjPARTICLETILE", "uint8 quadrant", 0);

		engine->RegisterObjectMethod("jjPARTICLETILE", "uint16 get_tileID() const", asMETHOD(TparticleTILE, get_AStile), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPARTICLETILE", "uint16 set_tileID(uint16)", asMETHOD(TparticleTILE, set_AStile), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjPARTICLE", "jjPARTICLETILE tile", asOFFSET(Tparticle, GENERIC));

		engine->RegisterObjectType("jjCONTROLPOINT", sizeof(_controlPoint), asOBJ_REF | asOBJ_NOCOUNT);
		engine->RegisterGlobalFunction("const jjCONTROLPOINT@ get_jjControlPoints(int)", asFUNCTION(getControlPoint), asCALL_CDECL);
		engine->RegisterObjectProperty("jjCONTROLPOINT", "const string name", asOFFSET(_controlPoint, name));
		engine->RegisterObjectProperty("jjCONTROLPOINT", "const int xTile", asOFFSET(_controlPoint, xTile));
		engine->RegisterObjectProperty("jjCONTROLPOINT", "const int yTile", asOFFSET(_controlPoint, yTile));
		engine->RegisterObjectProperty("jjCONTROLPOINT", "const int direction", asOFFSET(_controlPoint, direction));
		engine->RegisterObjectProperty("jjCONTROLPOINT", "const TEAM::Color controlTeam", asOFFSET(_controlPoint, controlTeam));
		engine->RegisterObjectMethod("jjCONTROLPOINT", "float get_xPos() const", AS_OBJ_FLOAT_GETTER(_controlPoint, pos.x), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectMethod("jjCONTROLPOINT", "float get_yPos() const", AS_OBJ_FLOAT_GETTER(_controlPoint, pos.y), asCALL_CDECL_OBJLAST);*/

		engine->RegisterObjectType("jjSTREAM", sizeof(jjSTREAM), asOBJ_REF);
		engine->RegisterObjectBehaviour("jjSTREAM", asBEHAVE_FACTORY, "jjSTREAM@ f()", asFUNCTION(jjSTREAM::Create), asCALL_CDECL);
		engine->RegisterObjectBehaviour("jjSTREAM", asBEHAVE_FACTORY, "jjSTREAM@ f(const ::string &in filename)", asFUNCTION(jjSTREAM::CreateFromFile), asCALL_CDECL);
		engine->RegisterObjectBehaviour("jjSTREAM", asBEHAVE_ADDREF, "void f()", asMETHOD(jjSTREAM, AddRef), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("jjSTREAM", asBEHAVE_RELEASE, "void f()", asMETHOD(jjSTREAM, Release), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "jjSTREAM& opAssign(const jjSTREAM &in)", asMETHODPR(jjSTREAM, operator=, (const jjSTREAM& other), jjSTREAM&), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "uint getSize() const", asMETHOD(jjSTREAM, getSize), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool isEmpty() const", asMETHOD(jjSTREAM, isEmpty), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool save(const ::string &in filename) const", asMETHOD(jjSTREAM, save), asCALL_THISCALL);

		engine->RegisterObjectMethod("jjSTREAM", "void clear()", asMETHOD(jjSTREAM, clear), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool discard(uint count)", asMETHOD(jjSTREAM, discard), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool write(const ::string &in value)", asMETHODPR(jjSTREAM, write, (const String&), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool write(const jjSTREAM &in value)", asMETHODPR(jjSTREAM, write, (const jjSTREAM&), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool get(::string &out value, uint count = 1)", asMETHODPR(jjSTREAM, get, (String&, uint32_t), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool get(jjSTREAM &out value, uint count = 1)", asMETHODPR(jjSTREAM, get, (jjSTREAM&, uint32_t), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool getLine(::string &out value, const ::string &in delim = '\\n')", asMETHOD(jjSTREAM, getLine), asCALL_THISCALL);

		engine->RegisterObjectMethod("jjSTREAM", "bool push(bool value)", asMETHODPR(jjSTREAM, push, (bool), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool push(uint8 value)", asMETHODPR(jjSTREAM, push, (uint8_t), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool push(int8 value)", asMETHODPR(jjSTREAM, push, (int8_t), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool push(uint16 value)", asMETHODPR(jjSTREAM, push, (uint16_t), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool push(int16 value)", asMETHODPR(jjSTREAM, push, (int16_t), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool push(uint32 value)", asMETHODPR(jjSTREAM, push, (uint32_t), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool push(int32 value)", asMETHODPR(jjSTREAM, push, (int32_t), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool push(uint64 value)", asMETHODPR(jjSTREAM, push, (uint64_t), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool push(int64 value)", asMETHODPR(jjSTREAM, push, (int64_t), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool push(float value)", asMETHODPR(jjSTREAM, push, (float), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool push(double value)", asMETHODPR(jjSTREAM, push, (double), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool push(const ::string &in value)", asMETHODPR(jjSTREAM, push, (const String&), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool push(const jjSTREAM &in value)", asMETHODPR(jjSTREAM, push, (const jjSTREAM&), bool), asCALL_THISCALL);

		engine->RegisterObjectMethod("jjSTREAM", "bool pop(bool &out value)", asMETHODPR(jjSTREAM, pop, (bool&), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool pop(uint8 &out value)", asMETHODPR(jjSTREAM, pop, (uint8_t&), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool pop(int8 &out value)", asMETHODPR(jjSTREAM, pop, (int64_t&), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool pop(uint16 &out value)", asMETHODPR(jjSTREAM, pop, (uint16_t&), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool pop(int16 &out value)", asMETHODPR(jjSTREAM, pop, (int16_t&), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool pop(uint32 &out value)", asMETHODPR(jjSTREAM, pop, (uint32_t&), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool pop(int32 &out value)", asMETHODPR(jjSTREAM, pop, (int32_t&), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool pop(uint64 &out value)", asMETHODPR(jjSTREAM, pop, (uint64_t&), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool pop(int64 &out value)", asMETHODPR(jjSTREAM, pop, (int64_t&), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool pop(float &out value)", asMETHODPR(jjSTREAM, pop, (float&), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool pop(double &out value)", asMETHODPR(jjSTREAM, pop, (double&), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool pop(::string &out value)", asMETHODPR(jjSTREAM, pop, (String&), bool), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjSTREAM", "bool pop(jjSTREAM &out value)", asMETHODPR(jjSTREAM, pop, (jjSTREAM&), bool), asCALL_THISCALL);

		// TODO
		/*engine->RegisterGlobalFunction("bool jjSendPacket(const jjSTREAM &in packet, int toClientID = 0, uint toScriptModuleID = ::jjScriptModuleID)", asFUNCTION(sendASPacket), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool jjTakeScreenshot(const string &in filename = '')", asFUNCTION(requestScreenshot), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool jjZlibCompress(const jjSTREAM &in input, jjSTREAM &out output)", asFUNCTION(streamCompress), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool jjZlibUncompress(const jjSTREAM &in input, jjSTREAM &out output, uint size)", asFUNCTION(streamUncompress), asCALL_CDECL);
		engine->RegisterGlobalFunction("uint jjCRC32(const jjSTREAM &in input, uint crc = 0)", asFUNCTION(streamCRC32), asCALL_CDECL);

		engine->RegisterObjectType("jjRNG", sizeof(jjRNG), asOBJ_REF);
		engine->RegisterObjectBehaviour("jjRNG", asBEHAVE_FACTORY, "jjRNG@ f(uint64 seed = 5489)", asFUNCTION(jjRNG::factory), asCALL_CDECL);
		engine->RegisterObjectBehaviour("jjRNG", asBEHAVE_ADDREF, "void f()", asMETHOD(jjRNG, addRef), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("jjRNG", asBEHAVE_RELEASE, "void f()", asMETHOD(jjRNG, release), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjRNG", "uint64 opCall()", asMETHOD(jjRNG, operator()), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjRNG", "jjRNG& opAssign(const jjRNG &in)", asMETHOD(jjRNG, operator=), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjRNG", "bool opEquals(const jjRNG &in) const", asMETHOD(jjRNG, operator==), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjRNG", "void seed(uint64 value = 5489)", asMETHOD(jjRNG, seed), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjRNG", "void discard(uint64 count = 1)", asMETHOD(jjRNG, discard), asCALL_THISCALL);*/

		engine->RegisterInterface("jjPUBLICINTERFACE");
		engine->RegisterInterfaceMethod("jjPUBLICINTERFACE", "string getVersion() const");
		// TODO
		//engine->RegisterGlobalFunction("jjPUBLICINTERFACE@ jjGetPublicInterface(const string &in moduleName)", asFUNCTION(getPublicInterface), asCALL_CDECL);

		engine->RegisterObjectType("jjANIMFRAME", sizeof(jjANIMFRAME), asOBJ_REF /*| asOBJ_NOCOUNT*/);
		engine->RegisterObjectBehaviour("jjANIMFRAME", asBEHAVE_ADDREF, "void f()", asMETHOD(jjANIMFRAME, AddRef), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("jjANIMFRAME", asBEHAVE_RELEASE, "void f()", asMETHOD(jjANIMFRAME, Release), asCALL_THISCALL);
		engine->RegisterGlobalFunction("jjANIMFRAME @get_jjAnimFrames(uint)", asFUNCTION(jjANIMFRAME::get_jjAnimFrames), asCALL_CDECL);
		engine->RegisterObjectProperty("jjANIMFRAME", "int16 hotSpotX", asOFFSET(jjANIMFRAME, hotSpotX));
		engine->RegisterObjectProperty("jjANIMFRAME", "int16 hotSpotY", asOFFSET(jjANIMFRAME, hotSpotY));
		engine->RegisterObjectProperty("jjANIMFRAME", "int16 coldSpotX", asOFFSET(jjANIMFRAME, coldSpotX));
		engine->RegisterObjectProperty("jjANIMFRAME", "int16 coldSpotY", asOFFSET(jjANIMFRAME, coldSpotY));
		engine->RegisterObjectProperty("jjANIMFRAME", "int16 gunSpotX", asOFFSET(jjANIMFRAME, gunSpotX));
		engine->RegisterObjectProperty("jjANIMFRAME", "int16 gunSpotY", asOFFSET(jjANIMFRAME, gunSpotY));
		engine->RegisterObjectProperty("jjANIMFRAME", "const uint16 width", asOFFSET(jjANIMFRAME, width));
		engine->RegisterObjectProperty("jjANIMFRAME", "const uint16 height", asOFFSET(jjANIMFRAME, height));
		engine->RegisterObjectMethod("jjANIMFRAME", "jjANIMFRAME& opAssign(const jjANIMFRAME &in)", asMETHOD(jjANIMFRAME, operator=), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjANIMFRAME", "bool get_transparent() const", asMETHOD(jjANIMFRAME, get_transparent), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjANIMFRAME", "bool set_transparent(bool)", asMETHOD(jjANIMFRAME, set_transparent), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjANIMFRAME", "bool doesCollide(int xPos, int yPos, int direction, const jjANIMFRAME@ frame2, int xPos2, int yPos2, int direction2, bool always = false) const", asMETHOD(jjANIMFRAME, doesCollide), asCALL_THISCALL);
		engine->RegisterObjectType("jjANIMATION", sizeof(jjANIMATION), asOBJ_REF /*| asOBJ_NOCOUNT*/);
		engine->RegisterObjectBehaviour("jjANIMATION", asBEHAVE_ADDREF, "void f()", asMETHOD(jjANIMATION, AddRef), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("jjANIMATION", asBEHAVE_RELEASE, "void f()", asMETHOD(jjANIMATION, Release), asCALL_THISCALL);
		engine->RegisterGlobalFunction("jjANIMATION @get_jjAnimations(uint)", asFUNCTION(jjANIMATION::get_jjAnimations), asCALL_CDECL);
		engine->RegisterObjectProperty("jjANIMATION", "uint16 frameCount", asOFFSET(jjANIMATION, frameCount));
		engine->RegisterObjectProperty("jjANIMATION", "int16 fps", asOFFSET(jjANIMATION, fps));
		engine->RegisterObjectMethod("jjANIMATION", "uint get_firstFrame() const", asMETHOD(jjANIMATION, get_firstFrame), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjANIMATION", "uint set_firstFrame(uint)", asMETHOD(jjANIMATION, set_firstFrame), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjANIMATION", "uint opImplConv() const", asMETHOD(jjANIMATION, getAnimFirstFrame), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjANIMATION", "jjANIMATION& opAssign(const jjANIMATION &in)", asMETHOD(jjANIMATION, operator=), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjANIMATION", "bool save(const ::string &in filename, const jjPAL &in palette = jjPalette) const", asMETHOD(jjANIMATION, save), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjANIMATION", "bool load(const ::string &in filename, int hotSpotX, int hotSpotY, int coldSpotYOffset = 0, int firstFrameToOverwrite = -1)", asMETHOD(jjANIMATION, load), asCALL_THISCALL);

		engine->RegisterObjectType("jjANIMSET", sizeof(jjANIMSET), asOBJ_REF /*| asOBJ_NOCOUNT*/);
		engine->RegisterObjectBehaviour("jjANIMSET", asBEHAVE_ADDREF, "void f()", asMETHOD(jjANIMSET, AddRef), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("jjANIMSET", asBEHAVE_RELEASE, "void f()", asMETHOD(jjANIMSET, Release), asCALL_THISCALL);
		engine->RegisterGlobalFunction("jjANIMSET @get_jjAnimSets(uint)", asFUNCTION(jjANIMSET::get_jjAnimSets), asCALL_CDECL);
		engine->RegisterObjectProperty("jjANIMSET", "uint firstAnim", 0);
		engine->RegisterObjectMethod("jjANIMSET", "uint opImplConv() const", asMETHOD(jjANIMSET, convertAnimSetToUint), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjANIMSET", "jjANIMSET @load(uint fileSetID = 2048, const string &in filename = '', int firstAnimToOverwrite = -1, int firstFrameToOverwrite = -1)", asMETHOD(jjANIMSET, load), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjANIMSET", "jjANIMSET @allocate(const array<uint> &in frameCounts)", asMETHOD(jjANIMSET, allocate), asCALL_THISCALL);

		engine->RegisterObjectMethod("jjCANVAS", "void drawString(int xPixel, int yPixel, const ::string &in text, const jjANIMATION &in animation, STRING::Mode mode = STRING::NORMAL, uint8 param = 0)", asMETHOD(jjCANVAS, drawString), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjCANVAS", "void drawString(int xPixel, int yPixel, const ::string &in text, const jjANIMATION &in animation, const jjTEXTAPPEARANCE &in appearance, uint8 param1 = 0, SPRITE::Mode spriteMode = SPRITE::PALSHIFT, uint8 param2 = 0)", asMETHOD(jjCANVAS, drawStringEx), asCALL_THISCALL);
		engine->RegisterGlobalFunction("void jjDrawString(float xPixel, float yPixel, const ::string &in text, const jjANIMATION &in animation, STRING::Mode mode = STRING::NORMAL, uint8 param = 0, int8 layerZ = 4, uint8 layerXY = 4, int8 playerID = -1)", asFUNCTION(jjCANVAS::jjDrawString), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjDrawString(float xPixel, float yPixel, const ::string &in text, const jjANIMATION &in animation, const jjTEXTAPPEARANCE &in appearance, uint8 param1 = 0, SPRITE::Mode spriteMode = SPRITE::PALSHIFT, uint8 param2 = 0, int8 layerZ = 4, uint8 layerXY = 4, int8 playerID = -1)", asFUNCTION(jjCANVAS::jjDrawStringEx), asCALL_CDECL);
		engine->RegisterGlobalFunction("int jjGetStringWidth(const ::string &in text, const jjANIMATION &in animation, const jjTEXTAPPEARANCE &in style)", asFUNCTION(jjCANVAS::jjGetStringWidth), asCALL_CDECL);

		engine->RegisterObjectType("jjLAYER", sizeof(jjLAYER), asOBJ_REF);

		engine->RegisterObjectType("jjPIXELMAP", sizeof(jjPIXELMAP), asOBJ_REF);
		engine->RegisterObjectBehaviour("jjPIXELMAP", asBEHAVE_FACTORY, "jjPIXELMAP@ f(uint16 tileID = 0)", asFUNCTION(jjPIXELMAP::CreateFromTile), asCALL_CDECL);
		engine->RegisterObjectBehaviour("jjPIXELMAP", asBEHAVE_FACTORY, "jjPIXELMAP@ f(uint width, uint height)", asFUNCTION(jjPIXELMAP::CreateFromSize), asCALL_CDECL);
		engine->RegisterObjectBehaviour("jjPIXELMAP", asBEHAVE_FACTORY, "jjPIXELMAP@ f(const jjANIMFRAME@ animFrame)", asFUNCTION(jjPIXELMAP::CreateFromFrame), asCALL_CDECL);
		engine->RegisterObjectBehaviour("jjPIXELMAP", asBEHAVE_FACTORY, "jjPIXELMAP@ f(uint left, uint top, uint width, uint height, uint layer = 4)", asFUNCTION(jjPIXELMAP::CreateFromLayer), asCALL_CDECL);
		engine->RegisterObjectBehaviour("jjPIXELMAP", asBEHAVE_FACTORY, "jjPIXELMAP@ f(uint left, uint top, uint width, uint height, const jjLAYER &in layer)", asFUNCTION(jjPIXELMAP::CreateFromLayerObject), asCALL_CDECL);
		engine->RegisterObjectBehaviour("jjPIXELMAP", asBEHAVE_FACTORY, "jjPIXELMAP@ f(TEXTURE::Texture texture)", asFUNCTION(jjPIXELMAP::CreateFromTexture), asCALL_CDECL);
		engine->RegisterObjectBehaviour("jjPIXELMAP", asBEHAVE_FACTORY, "jjPIXELMAP@ f(const ::string &in filename, const jjPAL &in palette = jjPalette, uint8 threshold = 1)", asFUNCTION(jjPIXELMAP::CreateFromFilename), asCALL_CDECL);
		engine->RegisterObjectBehaviour("jjPIXELMAP", asBEHAVE_ADDREF, "void f()", asMETHOD(jjPIXELMAP, AddRef), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("jjPIXELMAP", asBEHAVE_RELEASE, "void f()", asMETHOD(jjPIXELMAP, Release), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPIXELMAP", "uint8& opIndex(uint, uint)", asMETHOD(jjPIXELMAP, GetPixel), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPIXELMAP", "const uint8& opIndex(uint, uint) const", asMETHOD(jjPIXELMAP, GetPixel), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjPIXELMAP", "const uint width", asOFFSET(jjPIXELMAP, width));
		engine->RegisterObjectProperty("jjPIXELMAP", "const uint height", asOFFSET(jjPIXELMAP, height));
		engine->RegisterObjectMethod("jjPIXELMAP", "bool save(uint16 tileID, bool hFlip = false) const", asMETHOD(jjPIXELMAP, saveToTile), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPIXELMAP", "bool save(jjANIMFRAME@ frame) const", asMETHOD(jjPIXELMAP, saveToFrame), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPIXELMAP", "bool save(const ::string &in filename, const jjPAL &in palette = jjPalette) const", asMETHOD(jjPIXELMAP, saveToFile), asCALL_THISCALL);
		// TODO
		/*engine->RegisterObjectMethod("jjPIXELMAP", "bool makeTexture(jjLAYER@ layer = null)", asMETHOD(PixelMap, saveToTexture), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPIXELMAP", "jjPIXELMAP& crop(uint, uint, uint, uint)", asMETHOD(PixelMap, crop), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPIXELMAP", "jjPIXELMAP& addBorders(int, int, int, int, uint8 = 0)", asMETHOD(PixelMap, addBorders), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPIXELMAP", "jjPIXELMAP& flip(SPRITE::Direction)", asMETHOD(PixelMap, flip), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPIXELMAP", "jjPIXELMAP& rotate()", asMETHOD(PixelMap, rotate), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPIXELMAP", "jjPIXELMAP& recolor(const array<uint8> &in)", asMETHOD(PixelMap, recolor), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPIXELMAP", "jjPIXELMAP& resize(uint, uint)", asMETHOD(PixelMap, resize), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPIXELMAP", "jjPIXELMAP& trim(uint &out, uint &out, uint &out, uint &out, uint8 = 0)", asMETHOD(PixelMap, trim), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPIXELMAP", "jjPIXELMAP& trim(uint8 = 0)", asMETHOD(PixelMap, trimBasic), asCALL_THISCALL);

		engine->RegisterObjectMethod("jjANIMSET", "jjANIMSET @load(const jjPIXELMAP &in, uint frameWidth, uint frameHeight, uint frameSpacingX = 0, uint frameSpacingY = 0, uint startX = 0, uint startY = 0, const array<int> &in coldSpotYOffsets = array<int>(), int firstAnimToOverwrite = -1, int firstFrameToOverwrite = -1)", asFUNCTION(importSpriteSheetToAnimSet), asCALL_CDECL_OBJFIRST);*/

		engine->RegisterObjectType("jjMASKMAP", sizeof(jjMASKMAP), asOBJ_REF);
		engine->RegisterObjectBehaviour("jjMASKMAP", asBEHAVE_FACTORY, "jjMASKMAP@ f(bool filled = false)", asFUNCTION(jjMASKMAP::CreateFromBool), asCALL_CDECL);
		engine->RegisterObjectBehaviour("jjMASKMAP", asBEHAVE_FACTORY, "jjMASKMAP@ f(uint16 tileID)", asFUNCTION(jjMASKMAP::CreateFromTile), asCALL_CDECL);
		engine->RegisterObjectBehaviour("jjMASKMAP", asBEHAVE_ADDREF, "void f()", asMETHOD(jjMASKMAP, AddRef), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("jjMASKMAP", asBEHAVE_RELEASE, "void f()", asMETHOD(jjMASKMAP, Release), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjMASKMAP", "bool& opIndex(uint, uint)", asMETHOD(jjMASKMAP, GetPixel), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjMASKMAP", "const bool& opIndex(uint, uint) const", asMETHOD(jjMASKMAP, GetPixel), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjMASKMAP", "bool save(uint16 tileID, bool hFlip = false) const", asMETHOD(jjMASKMAP, save), asCALL_THISCALL);

		engine->RegisterObjectBehaviour("jjLAYER", asBEHAVE_FACTORY, "jjLAYER@ f(uint layerWidth, uint layerHeight)", asFUNCTION(jjLAYER::CreateFromSize), asCALL_CDECL);
		engine->RegisterObjectBehaviour("jjLAYER", asBEHAVE_FACTORY, "jjLAYER@ f(const jjLAYER &in layer)", asFUNCTION(jjLAYER::CreateCopy), asCALL_CDECL);
		engine->RegisterObjectBehaviour("jjLAYER", asBEHAVE_ADDREF, "void f()", asMETHOD(jjLAYER, AddRef), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("jjLAYER", asBEHAVE_RELEASE, "void f()", asMETHOD(jjLAYER, Release), asCALL_THISCALL);
		engine->RegisterGlobalFunction("jjLAYER @get_jjLayers(int)", asFUNCTION(jjLAYER::get_jjLayers), asCALL_CDECL);
		engine->RegisterObjectProperty("jjLAYER", "const int width", asOFFSET(jjLAYER, width));
		engine->RegisterObjectProperty("jjLAYER", "const int widthReal", asOFFSET(jjLAYER, widthReal));
		engine->RegisterObjectProperty("jjLAYER", "const int widthRounded", asOFFSET(jjLAYER, widthRounded));
		engine->RegisterObjectProperty("jjLAYER", "const int height", asOFFSET(jjLAYER, height));
		engine->RegisterObjectProperty("jjLAYER", "float xSpeed", asOFFSET(jjLAYER, xSpeed));
		engine->RegisterObjectProperty("jjLAYER", "float ySpeed", asOFFSET(jjLAYER, ySpeed));
		engine->RegisterObjectProperty("jjLAYER", "float xAutoSpeed", asOFFSET(jjLAYER, xAutoSpeed));
		engine->RegisterObjectProperty("jjLAYER", "float yAutoSpeed", asOFFSET(jjLAYER, yAutoSpeed));
		engine->RegisterObjectProperty("jjLAYER", "float xOffset", asOFFSET(jjLAYER, xOffset));
		engine->RegisterObjectProperty("jjLAYER", "float yOffset", asOFFSET(jjLAYER, yOffset));
		engine->RegisterObjectProperty("jjLAYER", "float xInnerSpeed", asOFFSET(jjLAYER, xInnerSpeed));
		engine->RegisterObjectProperty("jjLAYER", "float yInnerSpeed", asOFFSET(jjLAYER, yInnerSpeed));
		engine->RegisterObjectProperty("jjLAYER", "float xInnerAutoSpeed", asOFFSET(jjLAYER, xInnerAutoSpeed));
		engine->RegisterObjectProperty("jjLAYER", "float yInnerAutoSpeed", asOFFSET(jjLAYER, yInnerAutoSpeed));
		engine->RegisterObjectMethod("jjLAYER", "SPRITE::Mode get_spriteMode() const", asMETHOD(jjLAYER, get_spriteMode), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYER", "SPRITE::Mode set_spriteMode(SPRITE::Mode)", asMETHOD(jjLAYER, set_spriteMode), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYER", "uint8 get_spriteParam() const", asMETHOD(jjLAYER, get_spriteParam), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYER", "uint8 set_spriteParam(uint8)", asMETHOD(jjLAYER, set_spriteParam), asCALL_THISCALL);

		engine->RegisterObjectMethod("jjLAYER", "void setXSpeed(float newspeed, bool newSpeedIsAnAutoSpeed)", asMETHOD(jjLAYER, setXSpeed), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYER", "void setYSpeed(float newspeed, bool newSpeedIsAnAutoSpeed)", asMETHOD(jjLAYER, setYSpeed), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYER", "float getXPosition(const jjPLAYER &in play) const", asMETHOD(jjLAYER, getXPosition), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYER", "float getYPosition(const jjPLAYER &in play) const", asMETHOD(jjLAYER, getYPosition), asCALL_THISCALL);

		// TODO
		/*engine->RegisterObjectType("jjLAYERWARPHORIZON", 0, asOBJ_REF | asOBJ_NOHANDLE);
		engine->RegisterObjectProperty("jjLAYERWARPHORIZON", "float fadePositionX", asOFFSET(jjLAYER, WARPHORIZON.FadePosition[0]));
		engine->RegisterObjectProperty("jjLAYERWARPHORIZON", "float fadePositionY", asOFFSET(jjLAYER, WARPHORIZON.FadePosition[1]));
		engine->RegisterObjectMethod("jjLAYERWARPHORIZON", "jjPALCOLOR getFadeColor() const", asMETHOD(jjLAYER, GetFadeColor), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERWARPHORIZON", "void setFadeColor(jjPALCOLOR)", asMETHOD(jjLAYER, SetFadeColor), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERWARPHORIZON", "bool get_stars() const", asMETHOD(jjLAYER, GetStars), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERWARPHORIZON", "bool set_stars(bool)", asMETHOD(jjLAYER, SetStars), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjLAYERWARPHORIZON", "bool fade", asOFFSET(jjLAYER, WARPHORIZON.Fade));
		engine->RegisterObjectProperty("jjLAYER", "jjLAYERWARPHORIZON warpHorizon", asOFFSET(jjLAYER, WARPHORIZON));
		engine->RegisterObjectType("jjLAYERTUNNEL", 0, asOBJ_REF | asOBJ_NOHANDLE);
		engine->RegisterObjectProperty("jjLAYERTUNNEL", "float fadePositionX", asOFFSET(jjLAYER, TUNNEL.FadePosition[0]));
		engine->RegisterObjectProperty("jjLAYERTUNNEL", "float fadePositionY", asOFFSET(jjLAYER, TUNNEL.FadePosition[1]));
		engine->RegisterObjectMethod("jjLAYERTUNNEL", "jjPALCOLOR getFadeColor() const", asMETHOD(jjLAYER, GetFadeColor), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERTUNNEL", "void setFadeColor(jjPALCOLOR)", asMETHOD(jjLAYER, SetFadeColor), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERTUNNEL", "bool get_spiral() const", asMETHOD(jjLAYER, GetStars), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERTUNNEL", "bool set_spiral(bool)", asMETHOD(jjLAYER, SetStars), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjLAYERTUNNEL", "bool fade", asOFFSET(jjLAYER, TUNNEL.Fade));
		engine->RegisterObjectProperty("jjLAYER", "jjLAYERTUNNEL tunnel", asOFFSET(jjLAYER, TUNNEL));
		engine->RegisterObjectType("jjLAYERMENU", 0, asOBJ_REF | asOBJ_NOHANDLE);
		engine->RegisterObjectProperty("jjLAYERMENU", "float pivotX", asOFFSET(jjLAYER, MENU.Pivot[0]));
		engine->RegisterObjectProperty("jjLAYERMENU", "float pivotY", asOFFSET(jjLAYER, MENU.Pivot[1]));
		engine->RegisterObjectMethod("jjLAYERMENU", "uint8 get_palrow16() const", asMETHOD(jjLAYER, GetFadeComponent<0>), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERMENU", "uint8 get_palrow32() const", asMETHOD(jjLAYER, GetFadeComponent<1>), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERMENU", "uint8 get_palrow256() const", asMETHOD(jjLAYER, GetFadeComponent<2>), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERMENU", "uint8 set_palrow16(uint8)", asMETHOD(jjLAYER, SetFadeComponent<0>), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERMENU", "uint8 set_palrow32(uint8)", asMETHOD(jjLAYER, SetFadeComponent<1>), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERMENU", "uint8 set_palrow256(uint8)", asMETHOD(jjLAYER, SetFadeComponent<2>), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERMENU", "bool get_lightToDark() const", asMETHOD(jjLAYER, GetStars), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERMENU", "bool set_lightToDark(bool)", asMETHOD(jjLAYER, SetStars), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjLAYER", "jjLAYERMENU menu", asOFFSET(jjLAYER, MENU));
		engine->RegisterObjectType("jjLAYERTILEMENU", 0, asOBJ_REF | asOBJ_NOHANDLE);
		engine->RegisterObjectProperty("jjLAYERTILEMENU", "float pivotX", asOFFSET(jjLAYER, TILEMENU.Pivot[0]));
		engine->RegisterObjectProperty("jjLAYERTILEMENU", "float pivotY", asOFFSET(jjLAYER, TILEMENU.Pivot[1]));
		engine->RegisterObjectMethod("jjLAYERTILEMENU", "bool get_fullSize() const", asMETHOD(jjLAYER, GetStars), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERTILEMENU", "bool set_fullSize(bool)", asMETHOD(jjLAYER, SetStars), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjLAYER", "jjLAYERTILEMENU tileMenu", asOFFSET(jjLAYER, TILEMENU));
		engine->RegisterObjectType("jjLAYERWAVE", 0, asOBJ_REF | asOBJ_NOHANDLE);
		engine->RegisterObjectProperty("jjLAYERWAVE", "float amplitudeX", asOFFSET(jjLAYER, WAVE.Amplitude[0]));
		engine->RegisterObjectProperty("jjLAYERWAVE", "float amplitudeY", asOFFSET(jjLAYER, WAVE.Amplitude[1]));
		engine->RegisterObjectMethod("jjLAYERWAVE", "uint8 get_wavelengthX() const", asMETHOD(jjLAYER, GetFadeComponent<0>), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERWAVE", "uint8 get_wavelengthY() const", asMETHOD(jjLAYER, GetFadeComponent<1>), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERWAVE", "int8 get_waveSpeed() const", asMETHOD(jjLAYER, GetFadeComponent<2>), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERWAVE", "uint8 set_wavelengthX(uint8)", asMETHOD(jjLAYER, SetFadeComponent<0>), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERWAVE", "uint8 set_wavelengthY(uint8)", asMETHOD(jjLAYER, SetFadeComponent<1>), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERWAVE", "int8 set_waveSpeed(int8)", asMETHOD(jjLAYER, SetFadeComponent<2>), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERWAVE", "bool get_distortionAngle() const", asMETHOD(jjLAYER, GetStars), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERWAVE", "bool set_distortionAngle(bool)", asMETHOD(jjLAYER, SetStars), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjLAYER", "jjLAYERWAVE wave", asOFFSET(jjLAYER, WAVE));
		engine->RegisterObjectType("jjLAYERCYLINDER", 0, asOBJ_REF | asOBJ_NOHANDLE);
		engine->RegisterObjectProperty("jjLAYERCYLINDER", "float fadePositionX", asOFFSET(jjLAYER, CYLINDER.FadePosition[0]));
		engine->RegisterObjectProperty("jjLAYERCYLINDER", "float fadePositionY", asOFFSET(jjLAYER, CYLINDER.FadePosition[1]));
		engine->RegisterObjectMethod("jjLAYERCYLINDER", "jjPALCOLOR getFadeColor() const", asMETHOD(jjLAYER, GetFadeColor), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERCYLINDER", "void setFadeColor(jjPALCOLOR)", asMETHOD(jjLAYER, SetFadeColor), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERCYLINDER", "bool get_halfSize() const", asMETHOD(jjLAYER, GetStars), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERCYLINDER", "bool set_halfSize(bool)", asMETHOD(jjLAYER, SetStars), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjLAYERCYLINDER", "bool fade", asOFFSET(jjLAYER, CYLINDER.Fade));
		engine->RegisterObjectProperty("jjLAYER", "jjLAYERCYLINDER cylinder", asOFFSET(jjLAYER, CYLINDER));
		engine->RegisterObjectType("jjLAYERREFLECTION", 0, asOBJ_REF | asOBJ_NOHANDLE);
		engine->RegisterObjectProperty("jjLAYERREFLECTION", "float fadePositionX", asOFFSET(jjLAYER, REFLECTION.FadePositionX));
		engine->RegisterObjectProperty("jjLAYERREFLECTION", "float top", asOFFSET(jjLAYER, REFLECTION.Top));
		engine->RegisterObjectMethod("jjLAYERREFLECTION", "uint8 get_tintOpacity() const", asMETHOD(jjLAYER, GetFadeComponent<0>), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERREFLECTION", "uint8 get_distance() const", asMETHOD(jjLAYER, GetFadeComponent<1>), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERREFLECTION", "uint8 get_distortion() const", asMETHOD(jjLAYER, GetFadeComponent<2>), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERREFLECTION", "uint8 set_tintOpacity(uint8)", asMETHOD(jjLAYER, SetFadeComponent<0>), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERREFLECTION", "uint8 set_distance(uint8)", asMETHOD(jjLAYER, SetFadeComponent<1>), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERREFLECTION", "uint8 set_distortion(uint8)", asMETHOD(jjLAYER, SetFadeComponent<2>), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERREFLECTION", "bool get_blur() const", asMETHOD(jjLAYER, GetStars), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYERREFLECTION", "bool set_blur(bool)", asMETHOD(jjLAYER, SetStars), asCALL_THISCALL);
		engine->RegisterObjectProperty("jjLAYERREFLECTION", "uint8 tintColor", asOFFSET(jjLAYER, REFLECTION.OverlayColor));
		engine->RegisterObjectProperty("jjLAYER", "jjLAYERREFLECTION reflection", asOFFSET(jjLAYER, REFLECTION));

		engine->RegisterObjectMethod("jjLAYER", "TEXTURE::Style get_textureStyle() const", asMETHOD(jjLAYER, GetTextureMode), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYER", "void set_textureStyle(TEXTURE::Style)", asMETHOD(jjLAYER, SetTextureMode), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYER", "TEXTURE::Texture get_texture() const", asMETHOD(jjLAYER, GetTexture), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYER", "void set_texture(TEXTURE::Texture)", asMETHOD(jjLAYER, SetTexture), asCALL_THISCALL);*/

		engine->RegisterObjectProperty("jjLAYER", "int rotationAngle", asOFFSET(jjLAYER, rotationAngle));
		engine->RegisterObjectProperty("jjLAYER", "int rotationRadiusMultiplier", asOFFSET(jjLAYER, rotationRadiusMultiplier));
		engine->RegisterObjectProperty("jjLAYER", "bool tileHeight", asOFFSET(jjLAYER, tileHeight));
		engine->RegisterObjectProperty("jjLAYER", "bool tileWidth", asOFFSET(jjLAYER, tileWidth));
		engine->RegisterObjectProperty("jjLAYER", "bool limitVisibleRegion", asOFFSET(jjLAYER, limitVisibleRegion));
		engine->RegisterObjectProperty("jjLAYER", "const bool hasTileMap", asOFFSET(jjLAYER, hasTileMap));
		engine->RegisterObjectProperty("jjLAYER", "bool hasTiles", asOFFSET(jjLAYER, hasTiles));
		engine->RegisterGlobalFunction("array<jjLAYER@>@ jjLayerOrderGet()", asFUNCTION(jjLAYER::jjLayerOrderGet), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool jjLayerOrderSet(const array<jjLAYER@> &in order)", asFUNCTION(jjLAYER::jjLayerOrderSet), asCALL_CDECL);
		engine->RegisterGlobalFunction("array<jjLAYER@>@ jjLayersFromLevel(const string &in filename, const array<uint> &in layerIDs, int tileIDAdjustmentFactor = 0)", asFUNCTION(jjLAYER::jjLayersFromLevel), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool jjTilesFromTileset(const string &in filename, uint firstTileID, uint tileCount, const array<uint8>@ paletteColorMapping = null)", asFUNCTION(jjLAYER::jjTilesFromTileset), asCALL_CDECL);

		// TODO
		/*engine->SetDefaultNamespace("LAYERSPEEDMODEL");
		engine->RegisterEnum("LayerSpeedModel");
		engine->RegisterEnumValue("LayerSpeedModel", "NORMAL", (int)jjLAYER::SpeedMode::Normal);
		engine->RegisterEnumValue("LayerSpeedModel", "LAYER8", (int)jjLAYER::SpeedMode::Layer8);
		engine->RegisterEnumValue("LayerSpeedModel", "BOTHSPEEDS", (int)jjLAYER::SpeedMode::BothRelativeAndAutoSpeeds);
		engine->RegisterEnumValue("LayerSpeedModel", "FROMSTART", (int)jjLAYER::SpeedMode::BothSpeedsButStuckToTopLeftCorner);
		engine->RegisterEnumValue("LayerSpeedModel", "FITLEVEL", (int)jjLAYER::SpeedMode::FitToLevelAndWindowSize);
		engine->RegisterEnumValue("LayerSpeedModel", "SPEEDMULTIPLIERS", (int)jjLAYER::SpeedMode::SpeedsAsPercentagesOfWindowSize);
		engine->SetDefaultNamespace("");
		engine->RegisterObjectProperty("jjLAYER", "LAYERSPEEDMODEL::LayerSpeedModel xSpeedModel", asOFFSET(jjLAYER, SpeedModeX));
		engine->RegisterObjectProperty("jjLAYER", "LAYERSPEEDMODEL::LayerSpeedModel ySpeedModel", asOFFSET(jjLAYER, SpeedModeY));

		engine->SetDefaultNamespace("SURFACE");
		engine->RegisterEnum("Surface");
		engine->RegisterEnumValue("Surface", "UNTEXTURED", jjLAYER::TextureSurfaceStyles::Untextured);
		engine->RegisterEnumValue("Surface", "LEGACY", jjLAYER::TextureSurfaceStyles::Legacy);
		engine->RegisterEnumValue("Surface", "FULLSCREEN", jjLAYER::TextureSurfaceStyles::FullScreen);
		engine->RegisterEnumValue("Surface", "INNERWINDOW", jjLAYER::TextureSurfaceStyles::InnerWindow);
		engine->RegisterEnumValue("Surface", "INNERLAYER", jjLAYER::TextureSurfaceStyles::InnerLayer);
		engine->SetDefaultNamespace("");
		engine->RegisterObjectMethod("jjLAYER", "SURFACE::Surface get_textureSurface() const", asMETHOD(jjLAYER, GetTextureSurface), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjLAYER", "void set_textureSurface(SURFACE::Surface)", asMETHOD(jjLAYER, SetTextureSurface), asCALL_THISCALL);

		engine->RegisterObjectType("jjTILE", sizeof(jjTILE), asOBJ_REF);
		engine->RegisterObjectBehaviour("jjTILE", asBEHAVE_ADDREF, "void f()", asMETHOD(jjTILE, addRef), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("jjTILE", asBEHAVE_RELEASE, "void f()", asMETHOD(jjTILE, release), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjTILE", "array<uint16>@ getFrames() const", asMETHOD(jjTILE, getFrames), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjTILE", "bool setFrames(const array<uint16> &in frames, bool pingPong = false, uint16 wait = 0, uint16 randomWait = 0, uint16 pingPongWait = 0)", asMETHOD(jjTILE, setFrames), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjTILE", "uint8 get_fps() const", asMETHOD(jjTILE, getFPS), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjTILE", "void set_fps(uint8)", asMETHOD(jjTILE, setFPS), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjTILE", "uint16 get_tileID() const", asMETHOD(jjTILE, getTileID), asCALL_THISCALL);

		engine->RegisterGlobalFunction("const jjTILE@ get_jjTiles(uint16)", asFUNCTION(jjTILE::getTile), asCALL_CDECL);
		engine->RegisterGlobalFunction("jjTILE@ get_jjAnimatedTiles(uint16)", asFUNCTION(jjTILE::getAnimatedTile), asCALL_CDECL);*/

		engine->RegisterGlobalFunction("uint16 jjGetStaticTile(uint16 tileID)", asFUNCTION(jjGetStaticTile), asCALL_CDECL);
		engine->RegisterGlobalFunction("uint16 jjTileGet(uint8 layer, int xTile, int yTile)", asFUNCTION(jjTileGet), asCALL_CDECL);
		engine->RegisterGlobalFunction("uint16 jjTileSet(uint8 layer, int xTile, int yTile, uint16 newTile)", asFUNCTION(jjTileSet), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjGenerateSettableTileArea(uint8 layer, int xTile, int yTile, int width, int height)", asFUNCTION(jjGenerateSettableTileArea), asCALL_CDECL);
		/*engine->RegisterObjectMethod("jjLAYER", "uint16 tileGet(int xTile, int yTile) const", asFUNCTION(getTileAtInLayer), asCALL_CDECL_OBJFIRST);
		engine->RegisterObjectMethod("jjLAYER", "uint16 tileSet(int xTile, int yTile, uint16 newTile)", asFUNCTION(setTileAtInLayer), asCALL_CDECL_OBJFIRST);
		engine->RegisterObjectMethod("jjLAYER", "void generateSettableTileArea(int xTile, int yTile, int width, int height)", asFUNCTION(generateSettableTileAreaInLayer), asCALL_CDECL_OBJFIRST);
		engine->RegisterObjectMethod("jjLAYER", "void generateSettableTileArea()", asFUNCTION(generateSettableLayer), asCALL_CDECL_OBJFIRST);*/

		engine->RegisterGlobalFunction("bool jjMaskedPixel(int xPixel, int yPixel)", asFUNCTION(jjMaskedPixel), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool jjMaskedPixel(int xPixel, int yPixel, uint8 layer)", asFUNCTION(jjMaskedPixelLayer), asCALL_CDECL);
		//engine->RegisterObjectMethod("jjLAYER", "bool maskedPixel(int xPixel, int yPixel) const", asMETHOD(jjLAYER, CheckPixel), asCALL_THISCALL);
		engine->RegisterGlobalFunction("bool jjMaskedHLine(int xPixel, int lineLength, int yPixel)", asFUNCTION(jjMaskedHLine), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool jjMaskedHLine(int xPixel, int lineLength, int yPixel, uint8 layer)", asFUNCTION(jjMaskedHLineLayer), asCALL_CDECL);
		//engine->RegisterObjectMethod("jjLAYER", "bool maskedHLine(int xPixel, int lineLength, int yPixel) const", asMETHOD(jjLAYER, CheckHLine), asCALL_THISCALL);
		engine->RegisterGlobalFunction("bool jjMaskedVLine(int xPixel, int yPixel, int lineLength)", asFUNCTION(jjMaskedVLine), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool jjMaskedVLine(int xPixel, int yPixel, int lineLength,uint8 layer)", asFUNCTION(jjMaskedVLineLayer), asCALL_CDECL);
		//engine->RegisterObjectMethod("jjLAYER", "bool maskedVLine(int xPixel, int yPixel, int lineLength) const", asMETHOD(jjLAYER, CheckVLineBool), asCALL_THISCALL);
		engine->RegisterGlobalFunction("int jjMaskedTopVLine(int xPixel, int yPixel, int lineLength)", asFUNCTION(jjMaskedTopVLine), asCALL_CDECL);
		engine->RegisterGlobalFunction("int jjMaskedTopVLine(int xPixel, int yPixel, int lineLength,uint8 layer)", asFUNCTION(jjMaskedTopVLineLayer), asCALL_CDECL);
		//engine->RegisterObjectMethod("jjLAYER", "int maskedTopVLine(int xPixel,int yPixel, int lineLength) const", asMETHOD(jjLAYER, CheckVLine), asCALL_THISCALL);
		//engine->RegisterGlobalProperty("uint8 jjEventAtLastMaskedPixel", tileAttr);

		engine->RegisterGlobalFunction("void jjSetModPosition(int order, int row, bool reset)", asFUNCTION(jjSetModPosition), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjSlideModChannelVolume(int channel, float volume, int milliseconds)", asFUNCTION(jjSlideModChannelVolume), asCALL_CDECL);
		engine->RegisterGlobalFunction("int jjGetModOrder()", asFUNCTION(jjGetModOrder), asCALL_CDECL);
		engine->RegisterGlobalFunction("int jjGetModRow()", asFUNCTION(jjGetModRow), asCALL_CDECL);
		engine->RegisterGlobalFunction("int jjGetModTempo()", asFUNCTION(jjGetModTempo), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjSetModTempo(uint8 tempo)", asFUNCTION(jjSetModTempo), asCALL_CDECL);
		engine->RegisterGlobalFunction("int jjGetModSpeed()", asFUNCTION(jjGetModSpeed), asCALL_CDECL);
		engine->RegisterGlobalFunction("void jjSetModSpeed(uint8 speed)", asFUNCTION(jjSetModSpeed), asCALL_CDECL);

		// TODO
		/*engine->RegisterObjectType("jjPLAYERDRAW", sizeof(DrawPlayerElements), asOBJ_REF | asOBJ_NOCOUNT);
		engine->RegisterObjectProperty("jjPLAYERDRAW", "bool name", asOFFSET(DrawPlayerElements, Name));
		engine->RegisterObjectProperty("jjPLAYERDRAW", "bool sprite", asOFFSET(DrawPlayerElements, Sprite));
		engine->RegisterObjectProperty("jjPLAYERDRAW", "bool sugarRush", asOFFSET(DrawPlayerElements, SugarRush));
		engine->RegisterObjectProperty("jjPLAYERDRAW", "bool gunFlash", asOFFSET(DrawPlayerElements, Flare));
		engine->RegisterObjectProperty("jjPLAYERDRAW", "bool invincibility", asOFFSET(DrawPlayerElements, Invincibility));
		engine->RegisterObjectProperty("jjPLAYERDRAW", "bool trail", asOFFSET(DrawPlayerElements, Trail));
		engine->RegisterObjectProperty("jjPLAYERDRAW", "bool morphingExplosions", asOFFSET(DrawPlayerElements, Morph));
		engine->RegisterObjectProperty("jjPLAYERDRAW", "bool airboardBouncingMotion", asOFFSET(DrawPlayerElements, AirboardOffset));
		engine->RegisterObjectProperty("jjPLAYERDRAW", "bool airboardPuff", asOFFSET(DrawPlayerElements, AirboardPuff));
		engine->RegisterObjectProperty("jjPLAYERDRAW", "SPRITE::Mode spriteMode", asOFFSET(DrawPlayerElements, SpriteMode));
		engine->RegisterObjectProperty("jjPLAYERDRAW", "uint8 spriteParam", asOFFSET(DrawPlayerElements, SpriteParam));
		engine->RegisterObjectProperty("jjPLAYERDRAW", "LIGHT::Type lightType", asOFFSET(DrawPlayerElements, LightType));
		engine->RegisterObjectProperty("jjPLAYERDRAW", "int8 light", asOFFSET(DrawPlayerElements, LightIntensity));
		engine->RegisterObjectProperty("jjPLAYERDRAW", "int layer", asOFFSET(DrawPlayerElements, Layer));
		engine->RegisterObjectProperty("jjPLAYERDRAW", "uint curFrame", asOFFSET(DrawPlayerElements, CurFrame));
		engine->RegisterObjectProperty("jjPLAYERDRAW", "int angle", asOFFSET(DrawPlayerElements, Angle));
		engine->RegisterObjectProperty("jjPLAYERDRAW", "float xOffset", asOFFSET(DrawPlayerElements, XOffset));
		engine->RegisterObjectProperty("jjPLAYERDRAW", "float yOffset", asOFFSET(DrawPlayerElements, YOffset));
		engine->RegisterObjectProperty("jjPLAYERDRAW", "float xScale", asOFFSET(DrawPlayerElements, XScale));
		engine->RegisterObjectProperty("jjPLAYERDRAW", "float yScale", asOFFSET(DrawPlayerElements, YScale));
		engine->RegisterObjectProperty("jjPLAYERDRAW", "TEAM::Color flag", asOFFSET(DrawPlayerElements, FlagTeam));
		engine->RegisterObjectMethod("jjPLAYERDRAW", "bool get_shield(SHIELD::Shield) const", asMETHOD(DrawPlayerElements, getShield1Index), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYERDRAW", "bool set_shield(SHIELD::Shield, bool)", asMETHOD(DrawPlayerElements, setShield1Index), asCALL_THISCALL);
		engine->RegisterObjectMethod("jjPLAYERDRAW", "jjPLAYER@ get_player() const", asMETHOD(DrawPlayerElements, getPlayer), asCALL_THISCALL);*/

		engine->SetDefaultNamespace("STATE");
		engine->RegisterEnumValue("State", "START", sSTART);
		engine->RegisterEnumValue("State", "SLEEP", sSLEEP);
		engine->RegisterEnumValue("State", "WAKE", sWAKE);
		engine->RegisterEnumValue("State", "KILL", sKILL);
		engine->RegisterEnumValue("State", "DEACTIVATE", sDEACTIVATE);
		engine->RegisterEnumValue("State", "WALK", sWALK);
		engine->RegisterEnumValue("State", "JUMP", sJUMP);
		engine->RegisterEnumValue("State", "FIRE", sFIRE);
		engine->RegisterEnumValue("State", "FLY", sFLY);
		engine->RegisterEnumValue("State", "BOUNCE", sBOUNCE);
		engine->RegisterEnumValue("State", "EXPLODE", sEXPLODE);
		engine->RegisterEnumValue("State", "ROCKETFLY", sROCKETFLY);
		engine->RegisterEnumValue("State", "STILL", sSTILL);
		engine->RegisterEnumValue("State", "FLOAT", sFLOAT);
		engine->RegisterEnumValue("State", "HIT", sHIT);
		engine->RegisterEnumValue("State", "SPRING", sSPRING);
		engine->RegisterEnumValue("State", "ACTION", sACTION);
		engine->RegisterEnumValue("State", "DONE", sDONE);
		engine->RegisterEnumValue("State", "PUSH", sPUSH);
		engine->RegisterEnumValue("State", "FALL", sFALL);
		engine->RegisterEnumValue("State", "FLOATFALL", sFLOATFALL);
		engine->RegisterEnumValue("State", "CIRCLE", sCIRCLE);
		engine->RegisterEnumValue("State", "ATTACK", sATTACK);
		engine->RegisterEnumValue("State", "FREEZE", sFREEZE);
		engine->RegisterEnumValue("State", "FADEIN", sFADEIN);
		engine->RegisterEnumValue("State", "FADEOUT", sFADEOUT);
		engine->RegisterEnumValue("State", "HIDE", sHIDE);
		engine->RegisterEnumValue("State", "TURN", sTURN);
		engine->RegisterEnumValue("State", "IDLE", sIDLE);
		engine->RegisterEnumValue("State", "EXTRA", sEXTRA);
		engine->RegisterEnumValue("State", "STOP", sSTOP);
		engine->RegisterEnumValue("State", "WAIT", sWAIT);
		engine->RegisterEnumValue("State", "LAND", sLAND);
		engine->RegisterEnumValue("State", "DELAYEDSTART", sDELAYEDSTART);
		engine->RegisterEnumValue("State", "ROTATE", sROTATE);
		engine->RegisterEnumValue("State", "DUCK", sDUCK);

		engine->SetDefaultNamespace("SOUND");
		engine->RegisterEnumValue("Sample", "AMMO_BLUB1", sAMMO_BLUB1);
		engine->RegisterEnumValue("Sample", "AMMO_BLUB2", sAMMO_BLUB2);
		engine->RegisterEnumValue("Sample", "AMMO_BMP1", sAMMO_BMP1);
		engine->RegisterEnumValue("Sample", "AMMO_BMP2", sAMMO_BMP2);
		engine->RegisterEnumValue("Sample", "AMMO_BMP3", sAMMO_BMP3);
		engine->RegisterEnumValue("Sample", "AMMO_BMP4", sAMMO_BMP4);
		engine->RegisterEnumValue("Sample", "AMMO_BMP5", sAMMO_BMP5);
		engine->RegisterEnumValue("Sample", "AMMO_BMP6", sAMMO_BMP6);
		engine->RegisterEnumValue("Sample", "AMMO_BOEM1", sAMMO_BOEM1);
		engine->RegisterEnumValue("Sample", "AMMO_BUL1", sAMMO_BUL1);
		engine->RegisterEnumValue("Sample", "AMMO_BULFL1", sAMMO_BULFL1);
		engine->RegisterEnumValue("Sample", "AMMO_BULFL2", sAMMO_BULFL2);
		engine->RegisterEnumValue("Sample", "AMMO_BULFL3", sAMMO_BULFL3);
		engine->RegisterEnumValue("Sample", "AMMO_FIREGUN1A", sAMMO_FIREGUN1A);
		engine->RegisterEnumValue("Sample", "AMMO_FIREGUN2A", sAMMO_FIREGUN2A);
		engine->RegisterEnumValue("Sample", "AMMO_FUMP", sAMMO_FUMP);
		engine->RegisterEnumValue("Sample", "AMMO_GUN1", sAMMO_GUN1);
		engine->RegisterEnumValue("Sample", "AMMO_GUN2", sAMMO_GUN2);
		engine->RegisterEnumValue("Sample", "AMMO_GUN3PLOP", sAMMO_GUN3PLOP);
		engine->RegisterEnumValue("Sample", "AMMO_GUNFLP", sAMMO_GUNFLP);
		engine->RegisterEnumValue("Sample", "AMMO_GUNFLP1", sAMMO_GUNFLP1);
		engine->RegisterEnumValue("Sample", "AMMO_GUNFLP2", sAMMO_GUNFLP2);
		engine->RegisterEnumValue("Sample", "AMMO_GUNFLP3", sAMMO_GUNFLP3);
		engine->RegisterEnumValue("Sample", "AMMO_GUNFLP4", sAMMO_GUNFLP4);
		engine->RegisterEnumValue("Sample", "AMMO_GUNFLPL", sAMMO_GUNFLPL);
		engine->RegisterEnumValue("Sample", "AMMO_GUNJAZZ", sAMMO_GUNJAZZ);
		engine->RegisterEnumValue("Sample", "AMMO_GUNVELOCITY", sAMMO_GUNVELOCITY);
		engine->RegisterEnumValue("Sample", "AMMO_ICEGUN", sAMMO_ICEGUN);
		engine->RegisterEnumValue("Sample", "AMMO_ICEGUN2", sAMMO_ICEGUN2);
		engine->RegisterEnumValue("Sample", "AMMO_ICEGUNPU", sAMMO_ICEGUNPU);
		engine->RegisterEnumValue("Sample", "AMMO_ICEPU1", sAMMO_ICEPU1);
		engine->RegisterEnumValue("Sample", "AMMO_ICEPU2", sAMMO_ICEPU2);
		engine->RegisterEnumValue("Sample", "AMMO_ICEPU3", sAMMO_ICEPU3);
		engine->RegisterEnumValue("Sample", "AMMO_ICEPU4", sAMMO_ICEPU4);
		engine->RegisterEnumValue("Sample", "AMMO_LASER", sAMMO_LASER);
		engine->RegisterEnumValue("Sample", "AMMO_LASER2", sAMMO_LASER2);
		engine->RegisterEnumValue("Sample", "AMMO_LASER3", sAMMO_LASER3);
		engine->RegisterEnumValue("Sample", "AMMO_LAZRAYS", sAMMO_LAZRAYS);
		engine->RegisterEnumValue("Sample", "AMMO_MISSILE", sAMMO_MISSILE);
		engine->RegisterEnumValue("Sample", "AMMO_SPZBL1", sAMMO_SPZBL1);
		engine->RegisterEnumValue("Sample", "AMMO_SPZBL2", sAMMO_SPZBL2);
		engine->RegisterEnumValue("Sample", "AMMO_SPZBL3", sAMMO_SPZBL3);
		engine->RegisterEnumValue("Sample", "BAT_BATFLY1", sBAT_BATFLY1);
		engine->RegisterEnumValue("Sample", "BILSBOSS_BILLAPPEAR", sBILSBOSS_BILLAPPEAR);
		engine->RegisterEnumValue("Sample", "BILSBOSS_FINGERSNAP", sBILSBOSS_FINGERSNAP);
		engine->RegisterEnumValue("Sample", "BILSBOSS_FIRE", sBILSBOSS_FIRE);
		engine->RegisterEnumValue("Sample", "BILSBOSS_FIRESTART", sBILSBOSS_FIRESTART);
		engine->RegisterEnumValue("Sample", "BILSBOSS_SCARY3", sBILSBOSS_SCARY3);
		engine->RegisterEnumValue("Sample", "BILSBOSS_THUNDER", sBILSBOSS_THUNDER);
		engine->RegisterEnumValue("Sample", "BILSBOSS_ZIP", sBILSBOSS_ZIP);
		engine->RegisterEnumValue("Sample", "BONUS_BONUS1", sBONUS_BONUS1);
		engine->RegisterEnumValue("Sample", "BONUS_BONUSBLUB", sBONUS_BONUSBLUB);
		engine->RegisterEnumValue("Sample", "BUBBA_BUBBABOUNCE1", sBUBBA_BUBBABOUNCE1);
		engine->RegisterEnumValue("Sample", "BUBBA_BUBBABOUNCE2", sBUBBA_BUBBABOUNCE2);
		engine->RegisterEnumValue("Sample", "BUBBA_BUBBAEXPLO", sBUBBA_BUBBAEXPLO);
		engine->RegisterEnumValue("Sample", "BUBBA_FROG2", sBUBBA_FROG2);
		engine->RegisterEnumValue("Sample", "BUBBA_FROG3", sBUBBA_FROG3);
		engine->RegisterEnumValue("Sample", "BUBBA_FROG4", sBUBBA_FROG4);
		engine->RegisterEnumValue("Sample", "BUBBA_FROG5", sBUBBA_FROG5);
		engine->RegisterEnumValue("Sample", "BUBBA_SNEEZE2", sBUBBA_SNEEZE2);
		engine->RegisterEnumValue("Sample", "BUBBA_TORNADOATTACK2", sBUBBA_TORNADOATTACK2);
		engine->RegisterEnumValue("Sample", "BUMBEE_BEELOOP", sBUMBEE_BEELOOP);
		engine->RegisterEnumValue("Sample", "CATERPIL_RIDOE", sCATERPIL_RIDOE);
		engine->RegisterEnumValue("Sample", "COMMON_AIRBOARD", sCOMMON_AIRBOARD);
		engine->RegisterEnumValue("Sample", "COMMON_AIRBTURN", sCOMMON_AIRBTURN);
		engine->RegisterEnumValue("Sample", "COMMON_AIRBTURN2", sCOMMON_AIRBTURN2);
		engine->RegisterEnumValue("Sample", "COMMON_BASE1", sCOMMON_BASE1);
		engine->RegisterEnumValue("Sample", "COMMON_BELL_FIRE", sCOMMON_BELL_FIRE);
		engine->RegisterEnumValue("Sample", "COMMON_BELL_FIRE2", sCOMMON_BELL_FIRE2);
		engine->RegisterEnumValue("Sample", "COMMON_BENZIN1", sCOMMON_BENZIN1);
		engine->RegisterEnumValue("Sample", "COMMON_BIRDFLY", sCOMMON_BIRDFLY);
		engine->RegisterEnumValue("Sample", "COMMON_BIRDFLY2", sCOMMON_BIRDFLY2);
		engine->RegisterEnumValue("Sample", "COMMON_BLOKPLOP", sCOMMON_BLOKPLOP);
		engine->RegisterEnumValue("Sample", "COMMON_BLUB1", sCOMMON_BLUB1);
		engine->RegisterEnumValue("Sample", "COMMON_BUBBLGN1", sCOMMON_BUBBLGN1);
		engine->RegisterEnumValue("Sample", "COMMON_BURN", sCOMMON_BURN);
		engine->RegisterEnumValue("Sample", "COMMON_BURNIN", sCOMMON_BURNIN);
		engine->RegisterEnumValue("Sample", "COMMON_CANSPS", sCOMMON_CANSPS);
		engine->RegisterEnumValue("Sample", "COMMON_CLOCK", sCOMMON_CLOCK);
		engine->RegisterEnumValue("Sample", "COMMON_COIN", sCOMMON_COIN);
		engine->RegisterEnumValue("Sample", "COMMON_COLLAPS", sCOMMON_COLLAPS);
		engine->RegisterEnumValue("Sample", "COMMON_CUP", sCOMMON_CUP);
		engine->RegisterEnumValue("Sample", "COMMON_DAMPED1", sCOMMON_DAMPED1);
		engine->RegisterEnumValue("Sample", "COMMON_DOWN", sCOMMON_DOWN);
		engine->RegisterEnumValue("Sample", "COMMON_DOWNFL2", sCOMMON_DOWNFL2);
		engine->RegisterEnumValue("Sample", "COMMON_DRINKSPAZZ1", sCOMMON_DRINKSPAZZ1);
		engine->RegisterEnumValue("Sample", "COMMON_DRINKSPAZZ2", sCOMMON_DRINKSPAZZ2);
		engine->RegisterEnumValue("Sample", "COMMON_DRINKSPAZZ3", sCOMMON_DRINKSPAZZ3);
		engine->RegisterEnumValue("Sample", "COMMON_DRINKSPAZZ4", sCOMMON_DRINKSPAZZ4);
		engine->RegisterEnumValue("Sample", "COMMON_EAT1", sCOMMON_EAT1);
		engine->RegisterEnumValue("Sample", "COMMON_EAT2", sCOMMON_EAT2);
		engine->RegisterEnumValue("Sample", "COMMON_EAT3", sCOMMON_EAT3);
		engine->RegisterEnumValue("Sample", "COMMON_EAT4", sCOMMON_EAT4);
		engine->RegisterEnumValue("Sample", "COMMON_ELECTRIC1", sCOMMON_ELECTRIC1);
		engine->RegisterEnumValue("Sample", "COMMON_ELECTRIC2", sCOMMON_ELECTRIC2);
		engine->RegisterEnumValue("Sample", "COMMON_ELECTRICHIT", sCOMMON_ELECTRICHIT);
		engine->RegisterEnumValue("Sample", "COMMON_EXPL_TNT", sCOMMON_EXPL_TNT);
		engine->RegisterEnumValue("Sample", "COMMON_EXPSM1", sCOMMON_EXPSM1);
		engine->RegisterEnumValue("Sample", "COMMON_FLAMER", sCOMMON_FLAMER);
		engine->RegisterEnumValue("Sample", "COMMON_FLAP", sCOMMON_FLAP);
		engine->RegisterEnumValue("Sample", "COMMON_FOEW1", sCOMMON_FOEW1);
		engine->RegisterEnumValue("Sample", "COMMON_FOEW2", sCOMMON_FOEW2);
		engine->RegisterEnumValue("Sample", "COMMON_FOEW3", sCOMMON_FOEW3);
		engine->RegisterEnumValue("Sample", "COMMON_FOEW4", sCOMMON_FOEW4);
		engine->RegisterEnumValue("Sample", "COMMON_FOEW5", sCOMMON_FOEW5);
		engine->RegisterEnumValue("Sample", "COMMON_GEMSMSH1", sCOMMON_GEMSMSH1);
		engine->RegisterEnumValue("Sample", "COMMON_GLASS2", sCOMMON_GLASS2);
		engine->RegisterEnumValue("Sample", "COMMON_GUNSM1", sCOMMON_GUNSM1);
		engine->RegisterEnumValue("Sample", "COMMON_HARP1", sCOMMON_HARP1);
		engine->RegisterEnumValue("Sample", "COMMON_HEAD", sCOMMON_HEAD);
		engine->RegisterEnumValue("Sample", "COMMON_HELI1", sCOMMON_HELI1);
		engine->RegisterEnumValue("Sample", "COMMON_HIBELL", sCOMMON_HIBELL);
		engine->RegisterEnumValue("Sample", "COMMON_HOLYFLUT", sCOMMON_HOLYFLUT);
		engine->RegisterEnumValue("Sample", "COMMON_HORN1", sCOMMON_HORN1);
		engine->RegisterEnumValue("Sample", "COMMON_ICECRUSH", sCOMMON_ICECRUSH);
		engine->RegisterEnumValue("Sample", "COMMON_IMPACT1", sCOMMON_IMPACT1);
		engine->RegisterEnumValue("Sample", "COMMON_IMPACT2", sCOMMON_IMPACT2);
		engine->RegisterEnumValue("Sample", "COMMON_IMPACT3", sCOMMON_IMPACT3);
		engine->RegisterEnumValue("Sample", "COMMON_IMPACT4", sCOMMON_IMPACT4);
		engine->RegisterEnumValue("Sample", "COMMON_IMPACT5", sCOMMON_IMPACT5);
		engine->RegisterEnumValue("Sample", "COMMON_IMPACT6", sCOMMON_IMPACT6);
		engine->RegisterEnumValue("Sample", "COMMON_IMPACT7", sCOMMON_IMPACT7);
		engine->RegisterEnumValue("Sample", "COMMON_IMPACT8", sCOMMON_IMPACT8);
		engine->RegisterEnumValue("Sample", "COMMON_IMPACT9", sCOMMON_IMPACT9);
		engine->RegisterEnumValue("Sample", "COMMON_ITEMTRE", sCOMMON_ITEMTRE);
		engine->RegisterEnumValue("Sample", "COMMON_JUMP", sCOMMON_JUMP);
		engine->RegisterEnumValue("Sample", "COMMON_JUMP2", sCOMMON_JUMP2);
		engine->RegisterEnumValue("Sample", "COMMON_LAND", sCOMMON_LAND);
		engine->RegisterEnumValue("Sample", "COMMON_LAND1", sCOMMON_LAND1);
		engine->RegisterEnumValue("Sample", "COMMON_LAND2", sCOMMON_LAND2);
		engine->RegisterEnumValue("Sample", "COMMON_LANDCAN1", sCOMMON_LANDCAN1);
		engine->RegisterEnumValue("Sample", "COMMON_LANDCAN2", sCOMMON_LANDCAN2);
		engine->RegisterEnumValue("Sample", "COMMON_LANDPOP", sCOMMON_LANDPOP);
		engine->RegisterEnumValue("Sample", "COMMON_LOADJAZZ", sCOMMON_LOADJAZZ);
		engine->RegisterEnumValue("Sample", "COMMON_LOADSPAZ", sCOMMON_LOADSPAZ);
		engine->RegisterEnumValue("Sample", "COMMON_METALHIT", sCOMMON_METALHIT);
		engine->RegisterEnumValue("Sample", "COMMON_MONITOR", sCOMMON_MONITOR);
		engine->RegisterEnumValue("Sample", "COMMON_NOCOIN", sCOMMON_NOCOIN);
		engine->RegisterEnumValue("Sample", "COMMON_PICKUP1", sCOMMON_PICKUP1);
		engine->RegisterEnumValue("Sample", "COMMON_PICKUPW1", sCOMMON_PICKUPW1);
		engine->RegisterEnumValue("Sample", "COMMON_PISTOL1", sCOMMON_PISTOL1);
		engine->RegisterEnumValue("Sample", "COMMON_PLOOP1", sCOMMON_PLOOP1);
		engine->RegisterEnumValue("Sample", "COMMON_PLOP1", sCOMMON_PLOP1);
		engine->RegisterEnumValue("Sample", "COMMON_PLOP2", sCOMMON_PLOP2);
		engine->RegisterEnumValue("Sample", "COMMON_PLOP3", sCOMMON_PLOP3);
		engine->RegisterEnumValue("Sample", "COMMON_PLOP4", sCOMMON_PLOP4);
		engine->RegisterEnumValue("Sample", "COMMON_PLOPKORK", sCOMMON_PLOPKORK);
		engine->RegisterEnumValue("Sample", "COMMON_PREEXPL1", sCOMMON_PREEXPL1);
		engine->RegisterEnumValue("Sample", "COMMON_PREHELI", sCOMMON_PREHELI);
		engine->RegisterEnumValue("Sample", "COMMON_REVUP", sCOMMON_REVUP);
		engine->RegisterEnumValue("Sample", "COMMON_RINGGUN", sCOMMON_RINGGUN);
		engine->RegisterEnumValue("Sample", "COMMON_RINGGUN2", sCOMMON_RINGGUN2);
		engine->RegisterEnumValue("Sample", "COMMON_SHIELD1", sCOMMON_SHIELD1);
		engine->RegisterEnumValue("Sample", "COMMON_SHIELD4", sCOMMON_SHIELD4);
		engine->RegisterEnumValue("Sample", "COMMON_SHIELD_ELEC", sCOMMON_SHIELD_ELEC);
		engine->RegisterEnumValue("Sample", "COMMON_SHLDOF3", sCOMMON_SHLDOF3);
		engine->RegisterEnumValue("Sample", "COMMON_SLIP", sCOMMON_SLIP);
		engine->RegisterEnumValue("Sample", "COMMON_SMASH", sCOMMON_SMASH);
		engine->RegisterEnumValue("Sample", "COMMON_SPLAT1", sCOMMON_SPLAT1);
		engine->RegisterEnumValue("Sample", "COMMON_SPLAT2", sCOMMON_SPLAT2);
		engine->RegisterEnumValue("Sample", "COMMON_SPLAT3", sCOMMON_SPLAT3);
		engine->RegisterEnumValue("Sample", "COMMON_SPLAT4", sCOMMON_SPLAT4);
		engine->RegisterEnumValue("Sample", "COMMON_SPLUT", sCOMMON_SPLUT);
		engine->RegisterEnumValue("Sample", "COMMON_SPRING1", sCOMMON_SPRING1);
		engine->RegisterEnumValue("Sample", "COMMON_STEAM", sCOMMON_STEAM);
		engine->RegisterEnumValue("Sample", "COMMON_STEP", sCOMMON_STEP);
		engine->RegisterEnumValue("Sample", "COMMON_STRETCH", sCOMMON_STRETCH);
		engine->RegisterEnumValue("Sample", "COMMON_SWISH1", sCOMMON_SWISH1);
		engine->RegisterEnumValue("Sample", "COMMON_SWISH2", sCOMMON_SWISH2);
		engine->RegisterEnumValue("Sample", "COMMON_SWISH3", sCOMMON_SWISH3);
		engine->RegisterEnumValue("Sample", "COMMON_SWISH4", sCOMMON_SWISH4);
		engine->RegisterEnumValue("Sample", "COMMON_SWISH5", sCOMMON_SWISH5);
		engine->RegisterEnumValue("Sample", "COMMON_SWISH6", sCOMMON_SWISH6);
		engine->RegisterEnumValue("Sample", "COMMON_SWISH7", sCOMMON_SWISH7);
		engine->RegisterEnumValue("Sample", "COMMON_SWISH8", sCOMMON_SWISH8);
		engine->RegisterEnumValue("Sample", "COMMON_TELPORT1", sCOMMON_TELPORT1);
		engine->RegisterEnumValue("Sample", "COMMON_TELPORT2", sCOMMON_TELPORT2);
		engine->RegisterEnumValue("Sample", "COMMON_UP", sCOMMON_UP);
		engine->RegisterEnumValue("Sample", "COMMON_WATER", sCOMMON_WATER);
		engine->RegisterEnumValue("Sample", "COMMON_WOOD1", sCOMMON_WOOD1);
		engine->RegisterEnumValue("Sample", "DEMON_RUN", sDEMON_RUN);
		engine->RegisterEnumValue("Sample", "DEVILDEVAN_DRAGONFIRE", sDEVILDEVAN_DRAGONFIRE);
		engine->RegisterEnumValue("Sample", "DEVILDEVAN_FLAP", sDEVILDEVAN_FLAP);
		engine->RegisterEnumValue("Sample", "DEVILDEVAN_FROG4", sDEVILDEVAN_FROG4);
		engine->RegisterEnumValue("Sample", "DEVILDEVAN_JUMPUP", sDEVILDEVAN_JUMPUP);
		engine->RegisterEnumValue("Sample", "DEVILDEVAN_LAUGH", sDEVILDEVAN_LAUGH);
		engine->RegisterEnumValue("Sample", "DEVILDEVAN_PHASER2", sDEVILDEVAN_PHASER2);
		engine->RegisterEnumValue("Sample", "DEVILDEVAN_STRECH2", sDEVILDEVAN_STRECH2);
		engine->RegisterEnumValue("Sample", "DEVILDEVAN_STRECHTAIL", sDEVILDEVAN_STRECHTAIL);
		engine->RegisterEnumValue("Sample", "DEVILDEVAN_STRETCH1", sDEVILDEVAN_STRETCH1);
		engine->RegisterEnumValue("Sample", "DEVILDEVAN_STRETCH3", sDEVILDEVAN_STRETCH3);
		engine->RegisterEnumValue("Sample", "DEVILDEVAN_VANISH1", sDEVILDEVAN_VANISH1);
		engine->RegisterEnumValue("Sample", "DEVILDEVAN_WHISTLEDESCENDING2", sDEVILDEVAN_WHISTLEDESCENDING2);
		engine->RegisterEnumValue("Sample", "DEVILDEVAN_WINGSOUT", sDEVILDEVAN_WINGSOUT);
		engine->RegisterEnumValue("Sample", "DOG_AGRESSIV", sDOG_AGRESSIV);
		engine->RegisterEnumValue("Sample", "DOG_SNIF1", sDOG_SNIF1);
		engine->RegisterEnumValue("Sample", "DOG_WAF1", sDOG_WAF1);
		engine->RegisterEnumValue("Sample", "DOG_WAF2", sDOG_WAF2);
		engine->RegisterEnumValue("Sample", "DOG_WAF3", sDOG_WAF3);
		engine->RegisterEnumValue("Sample", "DRAGFLY_BEELOOP", sDRAGFLY_BEELOOP);
		engine->RegisterEnumValue("Sample", "ENDING_OHTHANK", sENDING_OHTHANK);
		engine->RegisterEnumValue("Sample", "ENDTUNEJAZZ_TUNE", sENDTUNEJAZZ_TUNE);
		engine->RegisterEnumValue("Sample", "ENDTUNELORI_CAKE", sENDTUNELORI_CAKE);
		engine->RegisterEnumValue("Sample", "ENDTUNESPAZ_TUNE", sENDTUNESPAZ_TUNE);
		engine->RegisterEnumValue("Sample", "EPICLOGO_EPIC1", sEPICLOGO_EPIC1);
		engine->RegisterEnumValue("Sample", "EPICLOGO_EPIC2", sEPICLOGO_EPIC2);
		engine->RegisterEnumValue("Sample", "EVA_KISS1", sEVA_KISS1);
		engine->RegisterEnumValue("Sample", "EVA_KISS2", sEVA_KISS2);
		engine->RegisterEnumValue("Sample", "EVA_KISS3", sEVA_KISS3);
		engine->RegisterEnumValue("Sample", "EVA_KISS4", sEVA_KISS4);
		engine->RegisterEnumValue("Sample", "FAN_FAN", sFAN_FAN);
		engine->RegisterEnumValue("Sample", "FATCHK_HIT1", sFATCHK_HIT1);
		engine->RegisterEnumValue("Sample", "FATCHK_HIT2", sFATCHK_HIT2);
		engine->RegisterEnumValue("Sample", "FATCHK_HIT3", sFATCHK_HIT3);
		engine->RegisterEnumValue("Sample", "FENCER_FENCE1", sFENCER_FENCE1);
		engine->RegisterEnumValue("Sample", "FROG_FROG", sFROG_FROG);
		engine->RegisterEnumValue("Sample", "FROG_FROG1", sFROG_FROG1);
		engine->RegisterEnumValue("Sample", "FROG_FROG2", sFROG_FROG2);
		engine->RegisterEnumValue("Sample", "FROG_FROG3", sFROG_FROG3);
		engine->RegisterEnumValue("Sample", "FROG_FROG4", sFROG_FROG4);
		engine->RegisterEnumValue("Sample", "FROG_FROG5", sFROG_FROG5);
		engine->RegisterEnumValue("Sample", "FROG_JAZZ2FROG", sFROG_JAZZ2FROG);
		engine->RegisterEnumValue("Sample", "FROG_TONG", sFROG_TONG);
		engine->RegisterEnumValue("Sample", "GLOVE_HIT", sGLOVE_HIT);
		engine->RegisterEnumValue("Sample", "HATTER_CUP", sHATTER_CUP);
		engine->RegisterEnumValue("Sample", "HATTER_HAT", sHATTER_HAT);
		engine->RegisterEnumValue("Sample", "HATTER_PTOEI", sHATTER_PTOEI);
		engine->RegisterEnumValue("Sample", "HATTER_SPLIN", sHATTER_SPLIN);
		engine->RegisterEnumValue("Sample", "HATTER_SPLOUT", sHATTER_SPLOUT);
		engine->RegisterEnumValue("Sample", "INTRO_BLOW", sINTRO_BLOW);
		engine->RegisterEnumValue("Sample", "INTRO_BOEM1", sINTRO_BOEM1);
		engine->RegisterEnumValue("Sample", "INTRO_BOEM2", sINTRO_BOEM2);
		engine->RegisterEnumValue("Sample", "INTRO_BRAKE", sINTRO_BRAKE);
		engine->RegisterEnumValue("Sample", "INTRO_END", sINTRO_END);
		engine->RegisterEnumValue("Sample", "INTRO_GRAB", sINTRO_GRAB);
		engine->RegisterEnumValue("Sample", "INTRO_GREN1", sINTRO_GREN1);
		engine->RegisterEnumValue("Sample", "INTRO_GREN2", sINTRO_GREN2);
		engine->RegisterEnumValue("Sample", "INTRO_GREN3", sINTRO_GREN3);
		engine->RegisterEnumValue("Sample", "INTRO_GUNM0", sINTRO_GUNM0);
		engine->RegisterEnumValue("Sample", "INTRO_GUNM1", sINTRO_GUNM1);
		engine->RegisterEnumValue("Sample", "INTRO_GUNM2", sINTRO_GUNM2);
		engine->RegisterEnumValue("Sample", "INTRO_HELI", sINTRO_HELI);
		engine->RegisterEnumValue("Sample", "INTRO_HITSPAZ", sINTRO_HITSPAZ);
		engine->RegisterEnumValue("Sample", "INTRO_HITTURT", sINTRO_HITTURT);
		engine->RegisterEnumValue("Sample", "INTRO_IFEEL", sINTRO_IFEEL);
		engine->RegisterEnumValue("Sample", "INTRO_INHALE", sINTRO_INHALE);
		engine->RegisterEnumValue("Sample", "INTRO_INSECT", sINTRO_INSECT);
		engine->RegisterEnumValue("Sample", "INTRO_KATROL", sINTRO_KATROL);
		engine->RegisterEnumValue("Sample", "INTRO_LAND", sINTRO_LAND);
		engine->RegisterEnumValue("Sample", "INTRO_MONSTER", sINTRO_MONSTER);
		engine->RegisterEnumValue("Sample", "INTRO_MONSTER2", sINTRO_MONSTER2);
		engine->RegisterEnumValue("Sample", "INTRO_ROCK", sINTRO_ROCK);
		engine->RegisterEnumValue("Sample", "INTRO_ROPE1", sINTRO_ROPE1);
		engine->RegisterEnumValue("Sample", "INTRO_ROPE2", sINTRO_ROPE2);
		engine->RegisterEnumValue("Sample", "INTRO_RUN", sINTRO_RUN);
		engine->RegisterEnumValue("Sample", "INTRO_SHOT1", sINTRO_SHOT1);
		engine->RegisterEnumValue("Sample", "INTRO_SHOTGRN", sINTRO_SHOTGRN);
		engine->RegisterEnumValue("Sample", "INTRO_SKI", sINTRO_SKI);
		engine->RegisterEnumValue("Sample", "INTRO_STRING", sINTRO_STRING);
		engine->RegisterEnumValue("Sample", "INTRO_SWISH1", sINTRO_SWISH1);
		engine->RegisterEnumValue("Sample", "INTRO_SWISH2", sINTRO_SWISH2);
		engine->RegisterEnumValue("Sample", "INTRO_SWISH3", sINTRO_SWISH3);
		engine->RegisterEnumValue("Sample", "INTRO_SWISH4", sINTRO_SWISH4);
		engine->RegisterEnumValue("Sample", "INTRO_UHTURT", sINTRO_UHTURT);
		engine->RegisterEnumValue("Sample", "INTRO_UP1", sINTRO_UP1);
		engine->RegisterEnumValue("Sample", "INTRO_UP2", sINTRO_UP2);
		engine->RegisterEnumValue("Sample", "INTRO_WIND_01", sINTRO_WIND_01);
		engine->RegisterEnumValue("Sample", "JAZZSOUNDS_BALANCE", sJAZZSOUNDS_BALANCE);
		engine->RegisterEnumValue("Sample", "JAZZSOUNDS_HEY1", sJAZZSOUNDS_HEY1);
		engine->RegisterEnumValue("Sample", "JAZZSOUNDS_HEY2", sJAZZSOUNDS_HEY2);
		engine->RegisterEnumValue("Sample", "JAZZSOUNDS_HEY3", sJAZZSOUNDS_HEY3);
		engine->RegisterEnumValue("Sample", "JAZZSOUNDS_HEY4", sJAZZSOUNDS_HEY4);
		engine->RegisterEnumValue("Sample", "JAZZSOUNDS_IDLE", sJAZZSOUNDS_IDLE);
		engine->RegisterEnumValue("Sample", "JAZZSOUNDS_JAZZV1", sJAZZSOUNDS_JAZZV1);
		engine->RegisterEnumValue("Sample", "JAZZSOUNDS_JAZZV2", sJAZZSOUNDS_JAZZV2);
		engine->RegisterEnumValue("Sample", "JAZZSOUNDS_JAZZV3", sJAZZSOUNDS_JAZZV3);
		engine->RegisterEnumValue("Sample", "JAZZSOUNDS_JAZZV4", sJAZZSOUNDS_JAZZV4);
		engine->RegisterEnumValue("Sample", "JAZZSOUNDS_JUMMY", sJAZZSOUNDS_JUMMY);
		engine->RegisterEnumValue("Sample", "JAZZSOUNDS_PFOE", sJAZZSOUNDS_PFOE);
		engine->RegisterEnumValue("Sample", "LABRAT_BITE", sLABRAT_BITE);
		engine->RegisterEnumValue("Sample", "LABRAT_EYE2", sLABRAT_EYE2);
		engine->RegisterEnumValue("Sample", "LABRAT_EYE3", sLABRAT_EYE3);
		engine->RegisterEnumValue("Sample", "LABRAT_MOUSE1", sLABRAT_MOUSE1);
		engine->RegisterEnumValue("Sample", "LABRAT_MOUSE2", sLABRAT_MOUSE2);
		engine->RegisterEnumValue("Sample", "LABRAT_MOUSE3", sLABRAT_MOUSE3);
		engine->RegisterEnumValue("Sample", "LIZARD_LIZ1", sLIZARD_LIZ1);
		engine->RegisterEnumValue("Sample", "LIZARD_LIZ2", sLIZARD_LIZ2);
		engine->RegisterEnumValue("Sample", "LIZARD_LIZ4", sLIZARD_LIZ4);
		engine->RegisterEnumValue("Sample", "LIZARD_LIZ6", sLIZARD_LIZ6);
		engine->RegisterEnumValue("Sample", "LORISOUNDS_DIE1", sLORISOUNDS_DIE1);
		engine->RegisterEnumValue("Sample", "LORISOUNDS_HURT0", sLORISOUNDS_HURT0);
		engine->RegisterEnumValue("Sample", "LORISOUNDS_HURT1", sLORISOUNDS_HURT1);
		engine->RegisterEnumValue("Sample", "LORISOUNDS_HURT2", sLORISOUNDS_HURT2);
		engine->RegisterEnumValue("Sample", "LORISOUNDS_HURT3", sLORISOUNDS_HURT3);
		engine->RegisterEnumValue("Sample", "LORISOUNDS_HURT4", sLORISOUNDS_HURT4);
		engine->RegisterEnumValue("Sample", "LORISOUNDS_HURT5", sLORISOUNDS_HURT5);
		engine->RegisterEnumValue("Sample", "LORISOUNDS_HURT6", sLORISOUNDS_HURT6);
		engine->RegisterEnumValue("Sample", "LORISOUNDS_HURT7", sLORISOUNDS_HURT7);
		engine->RegisterEnumValue("Sample", "LORISOUNDS_LORI1", sLORISOUNDS_LORI1);
		engine->RegisterEnumValue("Sample", "LORISOUNDS_LORI2", sLORISOUNDS_LORI2);
		engine->RegisterEnumValue("Sample", "LORISOUNDS_LORIBOOM", sLORISOUNDS_LORIBOOM);
		engine->RegisterEnumValue("Sample", "LORISOUNDS_LORIFALL", sLORISOUNDS_LORIFALL);
		engine->RegisterEnumValue("Sample", "LORISOUNDS_LORIJUMP", sLORISOUNDS_LORIJUMP);
		engine->RegisterEnumValue("Sample", "LORISOUNDS_LORIJUMP2", sLORISOUNDS_LORIJUMP2);
		engine->RegisterEnumValue("Sample", "LORISOUNDS_LORIJUMP3", sLORISOUNDS_LORIJUMP3);
		engine->RegisterEnumValue("Sample", "LORISOUNDS_LORIJUMP4", sLORISOUNDS_LORIJUMP4);
		engine->RegisterEnumValue("Sample", "LORISOUNDS_TOUCH", sLORISOUNDS_TOUCH);
		engine->RegisterEnumValue("Sample", "LORISOUNDS_WEHOO", sLORISOUNDS_WEHOO);
		engine->RegisterEnumValue("Sample", "MENUSOUNDS_SELECT0", sMENUSOUNDS_SELECT0);
		engine->RegisterEnumValue("Sample", "MENUSOUNDS_SELECT1", sMENUSOUNDS_SELECT1);
		engine->RegisterEnumValue("Sample", "MENUSOUNDS_SELECT2", sMENUSOUNDS_SELECT2);
		engine->RegisterEnumValue("Sample", "MENUSOUNDS_SELECT3", sMENUSOUNDS_SELECT3);
		engine->RegisterEnumValue("Sample", "MENUSOUNDS_SELECT4", sMENUSOUNDS_SELECT4);
		engine->RegisterEnumValue("Sample", "MENUSOUNDS_SELECT5", sMENUSOUNDS_SELECT5);
		engine->RegisterEnumValue("Sample", "MENUSOUNDS_SELECT6", sMENUSOUNDS_SELECT6);
		engine->RegisterEnumValue("Sample", "MENUSOUNDS_TYPE", sMENUSOUNDS_TYPE);
		engine->RegisterEnumValue("Sample", "MENUSOUNDS_TYPEENTER", sMENUSOUNDS_TYPEENTER);
		engine->RegisterEnumValue("Sample", "MONKEY_SPLUT", sMONKEY_SPLUT);
		engine->RegisterEnumValue("Sample", "MONKEY_THROW", sMONKEY_THROW);
		engine->RegisterEnumValue("Sample", "MOTH_FLAPMOTH", sMOTH_FLAPMOTH);
		engine->RegisterEnumValue("Sample", "ORANGE_BOEML", sORANGE_BOEML);
		engine->RegisterEnumValue("Sample", "ORANGE_BOEMR", sORANGE_BOEMR);
		engine->RegisterEnumValue("Sample", "ORANGE_BUBBELSL", sORANGE_BUBBELSL);
		engine->RegisterEnumValue("Sample", "ORANGE_BUBBELSR", sORANGE_BUBBELSR);
		engine->RegisterEnumValue("Sample", "ORANGE_GLAS1L", sORANGE_GLAS1L);
		engine->RegisterEnumValue("Sample", "ORANGE_GLAS1R", sORANGE_GLAS1R);
		engine->RegisterEnumValue("Sample", "ORANGE_GLAS2L", sORANGE_GLAS2L);
		engine->RegisterEnumValue("Sample", "ORANGE_GLAS2R", sORANGE_GLAS2R);
		engine->RegisterEnumValue("Sample", "ORANGE_MERGE", sORANGE_MERGE);
		engine->RegisterEnumValue("Sample", "ORANGE_SWEEP0L", sORANGE_SWEEP0L);
		engine->RegisterEnumValue("Sample", "ORANGE_SWEEP0R", sORANGE_SWEEP0R);
		engine->RegisterEnumValue("Sample", "ORANGE_SWEEP1L", sORANGE_SWEEP1L);
		engine->RegisterEnumValue("Sample", "ORANGE_SWEEP1R", sORANGE_SWEEP1R);
		engine->RegisterEnumValue("Sample", "ORANGE_SWEEP2L", sORANGE_SWEEP2L);
		engine->RegisterEnumValue("Sample", "ORANGE_SWEEP2R", sORANGE_SWEEP2R);
		engine->RegisterEnumValue("Sample", "P2_CRUNCH", sP2_CRUNCH);
		engine->RegisterEnumValue("Sample", "P2_FART", sP2_FART);
		engine->RegisterEnumValue("Sample", "P2_FOEW1", sP2_FOEW1);
		engine->RegisterEnumValue("Sample", "P2_FOEW4", sP2_FOEW4);
		engine->RegisterEnumValue("Sample", "P2_FOEW5", sP2_FOEW5);
		engine->RegisterEnumValue("Sample", "P2_FROG1", sP2_FROG1);
		engine->RegisterEnumValue("Sample", "P2_FROG2", sP2_FROG2);
		engine->RegisterEnumValue("Sample", "P2_FROG3", sP2_FROG3);
		engine->RegisterEnumValue("Sample", "P2_FROG4", sP2_FROG4);
		engine->RegisterEnumValue("Sample", "P2_FROG5", sP2_FROG5);
		engine->RegisterEnumValue("Sample", "P2_KISS4", sP2_KISS4);
		engine->RegisterEnumValue("Sample", "P2_OPEN", sP2_OPEN);
		engine->RegisterEnumValue("Sample", "P2_PINCH1", sP2_PINCH1);
		engine->RegisterEnumValue("Sample", "P2_PINCH2", sP2_PINCH2);
		engine->RegisterEnumValue("Sample", "P2_PLOPSEQ1", sP2_PLOPSEQ1);
		engine->RegisterEnumValue("Sample", "P2_PLOPSEQ2", sP2_PLOPSEQ2);
		engine->RegisterEnumValue("Sample", "P2_PLOPSEQ3", sP2_PLOPSEQ3);
		engine->RegisterEnumValue("Sample", "P2_PLOPSEQ4", sP2_PLOPSEQ4);
		engine->RegisterEnumValue("Sample", "P2_POEP", sP2_POEP);
		engine->RegisterEnumValue("Sample", "P2_PTOEI", sP2_PTOEI);
		engine->RegisterEnumValue("Sample", "P2_SPLOUT", sP2_SPLOUT);
		engine->RegisterEnumValue("Sample", "P2_SPLUT", sP2_SPLUT);
		engine->RegisterEnumValue("Sample", "P2_THROW", sP2_THROW);
		engine->RegisterEnumValue("Sample", "P2_TONG", sP2_TONG);
		engine->RegisterEnumValue("Sample", "PICKUPS_BOING_CHECK", sPICKUPS_BOING_CHECK);
		engine->RegisterEnumValue("Sample", "PICKUPS_HELI2", sPICKUPS_HELI2);
		engine->RegisterEnumValue("Sample", "PICKUPS_STRETCH1A", sPICKUPS_STRETCH1A);
		engine->RegisterEnumValue("Sample", "PINBALL_BELL", sPINBALL_BELL);
		engine->RegisterEnumValue("Sample", "PINBALL_FLIP1", sPINBALL_FLIP1);
		engine->RegisterEnumValue("Sample", "PINBALL_FLIP2", sPINBALL_FLIP2);
		engine->RegisterEnumValue("Sample", "PINBALL_FLIP3", sPINBALL_FLIP3);
		engine->RegisterEnumValue("Sample", "PINBALL_FLIP4", sPINBALL_FLIP4);
		engine->RegisterEnumValue("Sample", "QUEEN_LADYUP", sQUEEN_LADYUP);
		engine->RegisterEnumValue("Sample", "QUEEN_SCREAM", sQUEEN_SCREAM);
		engine->RegisterEnumValue("Sample", "RAPIER_GOSTDIE", sRAPIER_GOSTDIE);
		engine->RegisterEnumValue("Sample", "RAPIER_GOSTLOOP", sRAPIER_GOSTLOOP);
		engine->RegisterEnumValue("Sample", "RAPIER_GOSTOOOH", sRAPIER_GOSTOOOH);
		engine->RegisterEnumValue("Sample", "RAPIER_GOSTRIP", sRAPIER_GOSTRIP);
		engine->RegisterEnumValue("Sample", "RAPIER_HITCHAR", sRAPIER_HITCHAR);
		engine->RegisterEnumValue("Sample", "ROBOT_BIG1", sROBOT_BIG1);
		engine->RegisterEnumValue("Sample", "ROBOT_BIG2", sROBOT_BIG2);
		engine->RegisterEnumValue("Sample", "ROBOT_CAN1", sROBOT_CAN1);
		engine->RegisterEnumValue("Sample", "ROBOT_CAN2", sROBOT_CAN2);
		engine->RegisterEnumValue("Sample", "ROBOT_HYDRO", sROBOT_HYDRO);
		engine->RegisterEnumValue("Sample", "ROBOT_HYDRO2", sROBOT_HYDRO2);
		engine->RegisterEnumValue("Sample", "ROBOT_HYDROFIL", sROBOT_HYDROFIL);
		engine->RegisterEnumValue("Sample", "ROBOT_HYDROPUF", sROBOT_HYDROPUF);
		engine->RegisterEnumValue("Sample", "ROBOT_IDLE1", sROBOT_IDLE1);
		engine->RegisterEnumValue("Sample", "ROBOT_IDLE2", sROBOT_IDLE2);
		engine->RegisterEnumValue("Sample", "ROBOT_JMPCAN1", sROBOT_JMPCAN1);
		engine->RegisterEnumValue("Sample", "ROBOT_JMPCAN10", sROBOT_JMPCAN10);
		engine->RegisterEnumValue("Sample", "ROBOT_JMPCAN2", sROBOT_JMPCAN2);
		engine->RegisterEnumValue("Sample", "ROBOT_JMPCAN3", sROBOT_JMPCAN3);
		engine->RegisterEnumValue("Sample", "ROBOT_JMPCAN4", sROBOT_JMPCAN4);
		engine->RegisterEnumValue("Sample", "ROBOT_JMPCAN5", sROBOT_JMPCAN5);
		engine->RegisterEnumValue("Sample", "ROBOT_JMPCAN6", sROBOT_JMPCAN6);
		engine->RegisterEnumValue("Sample", "ROBOT_JMPCAN7", sROBOT_JMPCAN7);
		engine->RegisterEnumValue("Sample", "ROBOT_JMPCAN8", sROBOT_JMPCAN8);
		engine->RegisterEnumValue("Sample", "ROBOT_JMPCAN9", sROBOT_JMPCAN9);
		engine->RegisterEnumValue("Sample", "ROBOT_METAL1", sROBOT_METAL1);
		engine->RegisterEnumValue("Sample", "ROBOT_METAL2", sROBOT_METAL2);
		engine->RegisterEnumValue("Sample", "ROBOT_METAL3", sROBOT_METAL3);
		engine->RegisterEnumValue("Sample", "ROBOT_METAL4", sROBOT_METAL4);
		engine->RegisterEnumValue("Sample", "ROBOT_METAL5", sROBOT_METAL5);
		engine->RegisterEnumValue("Sample", "ROBOT_OPEN", sROBOT_OPEN);
		engine->RegisterEnumValue("Sample", "ROBOT_OUT", sROBOT_OUT);
		engine->RegisterEnumValue("Sample", "ROBOT_POEP", sROBOT_POEP);
		engine->RegisterEnumValue("Sample", "ROBOT_POLE", sROBOT_POLE);
		engine->RegisterEnumValue("Sample", "ROBOT_SHOOT", sROBOT_SHOOT);
		engine->RegisterEnumValue("Sample", "ROBOT_STEP1", sROBOT_STEP1);
		engine->RegisterEnumValue("Sample", "ROBOT_STEP2", sROBOT_STEP2);
		engine->RegisterEnumValue("Sample", "ROBOT_STEP3", sROBOT_STEP3);
		engine->RegisterEnumValue("Sample", "ROCK_ROCK1", sROCK_ROCK1);
		engine->RegisterEnumValue("Sample", "RUSH_RUSH", sRUSH_RUSH);
		engine->RegisterEnumValue("Sample", "SCIENCE_PLOPKAOS", sSCIENCE_PLOPKAOS);
		engine->RegisterEnumValue("Sample", "SKELETON_BONE1", sSKELETON_BONE1);
		engine->RegisterEnumValue("Sample", "SKELETON_BONE2", sSKELETON_BONE2);
		engine->RegisterEnumValue("Sample", "SKELETON_BONE3", sSKELETON_BONE3);
		engine->RegisterEnumValue("Sample", "SKELETON_BONE5", sSKELETON_BONE5);
		engine->RegisterEnumValue("Sample", "SKELETON_BONE6", sSKELETON_BONE6);
		engine->RegisterEnumValue("Sample", "SKELETON_BONE7", sSKELETON_BONE7);
		engine->RegisterEnumValue("Sample", "SMALTREE_FALL", sSMALTREE_FALL);
		engine->RegisterEnumValue("Sample", "SMALTREE_GROUND", sSMALTREE_GROUND);
		engine->RegisterEnumValue("Sample", "SMALTREE_HEAD", sSMALTREE_HEAD);
		engine->RegisterEnumValue("Sample", "SONCSHIP_METAL1", sSONCSHIP_METAL1);
		engine->RegisterEnumValue("Sample", "SONCSHIP_MISSILE2", sSONCSHIP_MISSILE2);
		engine->RegisterEnumValue("Sample", "SONCSHIP_SCRAPE", sSONCSHIP_SCRAPE);
		engine->RegisterEnumValue("Sample", "SONCSHIP_SHIPLOOP", sSONCSHIP_SHIPLOOP);
		engine->RegisterEnumValue("Sample", "SONCSHIP_TARGETLOCK", sSONCSHIP_TARGETLOCK);
		engine->RegisterEnumValue("Sample", "SONICSHIP_METAL1", sSONCSHIP_METAL1); // Private/deprecated
		engine->RegisterEnumValue("Sample", "SONICSHIP_MISSILE2", sSONCSHIP_MISSILE2); // Private/deprecated
		engine->RegisterEnumValue("Sample", "SONICSHIP_SCRAPE", sSONCSHIP_SCRAPE); // Private/deprecated
		engine->RegisterEnumValue("Sample", "SONICSHIP_SHIPLOOP", sSONCSHIP_SHIPLOOP); // Private/deprecated
		engine->RegisterEnumValue("Sample", "SONICSHIP_TARGETLOCK", sSONCSHIP_TARGETLOCK); // Private/deprecated
		engine->RegisterEnumValue("Sample", "SPAZSOUNDS_AUTSCH1", sSPAZSOUNDS_AUTSCH1);
		engine->RegisterEnumValue("Sample", "SPAZSOUNDS_AUTSCH2", sSPAZSOUNDS_AUTSCH2);
		engine->RegisterEnumValue("Sample", "SPAZSOUNDS_BIRDSIT", sSPAZSOUNDS_BIRDSIT);
		engine->RegisterEnumValue("Sample", "SPAZSOUNDS_BURP", sSPAZSOUNDS_BURP);
		engine->RegisterEnumValue("Sample", "SPAZSOUNDS_CHIRP", sSPAZSOUNDS_CHIRP);
		engine->RegisterEnumValue("Sample", "SPAZSOUNDS_EATBIRD", sSPAZSOUNDS_EATBIRD);
		engine->RegisterEnumValue("Sample", "SPAZSOUNDS_HAHAHA", sSPAZSOUNDS_HAHAHA);
		engine->RegisterEnumValue("Sample", "SPAZSOUNDS_HAHAHA2", sSPAZSOUNDS_HAHAHA2);
		engine->RegisterEnumValue("Sample", "SPAZSOUNDS_HAPPY", sSPAZSOUNDS_HAPPY);
		engine->RegisterEnumValue("Sample", "SPAZSOUNDS_HIHI", sSPAZSOUNDS_HIHI);
		engine->RegisterEnumValue("Sample", "SPAZSOUNDS_HOHOHO1", sSPAZSOUNDS_HOHOHO1);
		engine->RegisterEnumValue("Sample", "SPAZSOUNDS_HOOO", sSPAZSOUNDS_HOOO);
		engine->RegisterEnumValue("Sample", "SPAZSOUNDS_KARATE7", sSPAZSOUNDS_KARATE7);
		engine->RegisterEnumValue("Sample", "SPAZSOUNDS_KARATE8", sSPAZSOUNDS_KARATE8);
		engine->RegisterEnumValue("Sample", "SPAZSOUNDS_OHOH", sSPAZSOUNDS_OHOH);
		engine->RegisterEnumValue("Sample", "SPAZSOUNDS_OOOH", sSPAZSOUNDS_OOOH);
		engine->RegisterEnumValue("Sample", "SPAZSOUNDS_WOOHOO", sSPAZSOUNDS_WOOHOO);
		engine->RegisterEnumValue("Sample", "SPAZSOUNDS_YAHOO", sSPAZSOUNDS_YAHOO);
		engine->RegisterEnumValue("Sample", "SPAZSOUNDS_YAHOO2", sSPAZSOUNDS_YAHOO2);
		engine->RegisterEnumValue("Sample", "SPRING_BOING_DOWN", sSPRING_BOING_DOWN);
		engine->RegisterEnumValue("Sample", "SPRING_SPRING1", sSPRING_SPRING1);
		engine->RegisterEnumValue("Sample", "STEAM_STEAM", sSTEAM_STEAM);
		engine->RegisterEnumValue("Sample", "STONED_STONED", sSTONED_STONED);
		engine->RegisterEnumValue("Sample", "SUCKER_FART", sSUCKER_FART);
		engine->RegisterEnumValue("Sample", "SUCKER_PINCH1", sSUCKER_PINCH1);
		engine->RegisterEnumValue("Sample", "SUCKER_PINCH2", sSUCKER_PINCH2);
		engine->RegisterEnumValue("Sample", "SUCKER_PINCH3", sSUCKER_PINCH3);
		engine->RegisterEnumValue("Sample", "SUCKER_PLOPSEQ1", sSUCKER_PLOPSEQ1);
		engine->RegisterEnumValue("Sample", "SUCKER_PLOPSEQ2", sSUCKER_PLOPSEQ2);
		engine->RegisterEnumValue("Sample", "SUCKER_PLOPSEQ3", sSUCKER_PLOPSEQ3);
		engine->RegisterEnumValue("Sample", "SUCKER_PLOPSEQ4", sSUCKER_PLOPSEQ4);
		engine->RegisterEnumValue("Sample", "SUCKER_UP", sSUCKER_UP);
		engine->RegisterEnumValue("Sample", "TUFBOSS_CATCH", sTUFBOSS_CATCH);
		engine->RegisterEnumValue("Sample", "TUFBOSS_RELEASE", sTUFBOSS_RELEASE);
		engine->RegisterEnumValue("Sample", "TUFBOSS_SWING", sTUFBOSS_SWING);
		engine->RegisterEnumValue("Sample", "TURTLE_BITE3", sTURTLE_BITE3);
		engine->RegisterEnumValue("Sample", "TURTLE_HIDE", sTURTLE_HIDE);
		engine->RegisterEnumValue("Sample", "TURTLE_HITSHELL", sTURTLE_HITSHELL);
		engine->RegisterEnumValue("Sample", "TURTLE_IDLE1", sTURTLE_IDLE1);
		engine->RegisterEnumValue("Sample", "TURTLE_IDLE2", sTURTLE_IDLE2);
		engine->RegisterEnumValue("Sample", "TURTLE_NECK", sTURTLE_NECK);
		engine->RegisterEnumValue("Sample", "TURTLE_SPK1TURT", sTURTLE_SPK1TURT);
		engine->RegisterEnumValue("Sample", "TURTLE_SPK2TURT", sTURTLE_SPK2TURT);
		engine->RegisterEnumValue("Sample", "TURTLE_SPK3TURT", sTURTLE_SPK3TURT);
		engine->RegisterEnumValue("Sample", "TURTLE_SPK4TURT", sTURTLE_SPK4TURT);
		engine->RegisterEnumValue("Sample", "TURTLE_TURN", sTURTLE_TURN);
		engine->RegisterEnumValue("Sample", "UTERUS_CRABCLOSE", sUTERUS_CRABCLOSE);
		engine->RegisterEnumValue("Sample", "UTERUS_CRABOPEN2", sUTERUS_CRABOPEN2);
		engine->RegisterEnumValue("Sample", "UTERUS_SCISSORS1", sUTERUS_SCISSORS1);
		engine->RegisterEnumValue("Sample", "UTERUS_SCISSORS2", sUTERUS_SCISSORS2);
		engine->RegisterEnumValue("Sample", "UTERUS_SCISSORS3", sUTERUS_SCISSORS3);
		engine->RegisterEnumValue("Sample", "UTERUS_SCISSORS4", sUTERUS_SCISSORS4);
		engine->RegisterEnumValue("Sample", "UTERUS_SCISSORS5", sUTERUS_SCISSORS5);
		engine->RegisterEnumValue("Sample", "UTERUS_SCISSORS6", sUTERUS_SCISSORS6);
		engine->RegisterEnumValue("Sample", "UTERUS_SCISSORS7", sUTERUS_SCISSORS7);
		engine->RegisterEnumValue("Sample", "UTERUS_SCISSORS8", sUTERUS_SCISSORS8);
		engine->RegisterEnumValue("Sample", "UTERUS_SCREAM1", sUTERUS_SCREAM1);
		engine->RegisterEnumValue("Sample", "UTERUS_STEP1", sUTERUS_STEP1);
		engine->RegisterEnumValue("Sample", "UTERUS_STEP2", sUTERUS_STEP2);
		engine->RegisterEnumValue("Sample", "WIND_WIND2A", sWIND_WIND2A);
		engine->RegisterEnumValue("Sample", "WITCH_LAUGH", sWITCH_LAUGH);
		engine->RegisterEnumValue("Sample", "WITCH_MAGIC", sWITCH_MAGIC);
		engine->RegisterEnumValue("Sample", "XBILSY_BILLAPPEAR", sXBILSY_BILLAPPEAR);
		engine->RegisterEnumValue("Sample", "XBILSY_FINGERSNAP", sXBILSY_FINGERSNAP);
		engine->RegisterEnumValue("Sample", "XBILSY_FIRE", sXBILSY_FIRE);
		engine->RegisterEnumValue("Sample", "XBILSY_FIRESTART", sXBILSY_FIRESTART);
		engine->RegisterEnumValue("Sample", "XBILSY_SCARY3", sXBILSY_SCARY3);
		engine->RegisterEnumValue("Sample", "XBILSY_THUNDER", sXBILSY_THUNDER);
		engine->RegisterEnumValue("Sample", "XBILSY_ZIP", sXBILSY_ZIP);
		engine->RegisterEnumValue("Sample", "XLIZARD_LIZ1", sXLIZARD_LIZ1);
		engine->RegisterEnumValue("Sample", "XLIZARD_LIZ2", sXLIZARD_LIZ2);
		engine->RegisterEnumValue("Sample", "XLIZARD_LIZ4", sXLIZARD_LIZ4);
		engine->RegisterEnumValue("Sample", "XLIZARD_LIZ6", sXLIZARD_LIZ6);
		engine->RegisterEnumValue("Sample", "XTURTLE_BITE3", sXTURTLE_BITE3);
		engine->RegisterEnumValue("Sample", "XTURTLE_HIDE", sXTURTLE_HIDE);
		engine->RegisterEnumValue("Sample", "XTURTLE_HITSHELL", sXTURTLE_HITSHELL);
		engine->RegisterEnumValue("Sample", "XTURTLE_IDLE1", sXTURTLE_IDLE1);
		engine->RegisterEnumValue("Sample", "XTURTLE_IDLE2", sXTURTLE_IDLE2);
		engine->RegisterEnumValue("Sample", "XTURTLE_NECK", sXTURTLE_NECK);
		engine->RegisterEnumValue("Sample", "XTURTLE_SPK1TURT", sXTURTLE_SPK1TURT);
		engine->RegisterEnumValue("Sample", "XTURTLE_SPK2TURT", sXTURTLE_SPK2TURT);
		engine->RegisterEnumValue("Sample", "XTURTLE_SPK3TURT", sXTURTLE_SPK3TURT);
		engine->RegisterEnumValue("Sample", "XTURTLE_SPK4TURT", sXTURTLE_SPK4TURT);
		engine->RegisterEnumValue("Sample", "XTURTLE_TURN", sXTURTLE_TURN);
		engine->RegisterEnumValue("Sample", "ZDOG_AGRESSIV", sZDOG_AGRESSIV);
		engine->RegisterEnumValue("Sample", "ZDOG_SNIF1", sZDOG_SNIF1);
		engine->RegisterEnumValue("Sample", "ZDOG_WAF1", sZDOG_WAF1);
		engine->RegisterEnumValue("Sample", "ZDOG_WAF2", sZDOG_WAF2);
		engine->RegisterEnumValue("Sample", "ZDOG_WAF3", sZDOG_WAF3);

		engine->SetDefaultNamespace("AREA");
		engine->RegisterEnumValue("Area", "ONEWAY", areaONEWAY);
		engine->RegisterEnumValue("Area", "HURT", areaHURT);
		engine->RegisterEnumValue("Area", "VINE", areaVINE);
		engine->RegisterEnumValue("Area", "HOOK", areaHOOK);
		engine->RegisterEnumValue("Area", "SLIDE", areaSLIDE);
		engine->RegisterEnumValue("Area", "HPOLE", areaHPOLE);
		engine->RegisterEnumValue("Area", "VPOLE", areaVPOLE);
		engine->RegisterEnumValue("Area", "FLYOFF", areaFLYOFF);
		engine->RegisterEnumValue("Area", "RICOCHET", areaRICOCHET);
		engine->RegisterEnumValue("Area", "BELTRIGHT", areaBELTRIGHT);
		engine->RegisterEnumValue("Area", "BELTLEFT", areaBELTLEFT);
		engine->RegisterEnumValue("Area", "ACCBELTRIGHT", areaBELTACCRIGHT);
		engine->RegisterEnumValue("Area", "ACCBELTLEFT", areaBELTACCLEFT);
		engine->RegisterEnumValue("Area", "STOPENEMY", areaSTOPENEMY);
		engine->RegisterEnumValue("Area", "WINDLEFT", areaWINDLEFT);
		engine->RegisterEnumValue("Area", "WINDRIGHT", areaWINDRIGHT);
		engine->RegisterEnumValue("Area", "EOL", areaEOL);
		engine->RegisterEnumValue("Area", "WARPEOL", areaWARPEOL);
		engine->RegisterEnumValue("Area", "REVERTMORPH", areaENDMORPH);
		engine->RegisterEnumValue("Area", "FLOATUP", areaFLOATUP);
		engine->RegisterEnumValue("Area", "TRIGGERROCK", areaROCKTRIGGER);
		engine->RegisterEnumValue("Area", "DIMLIGHT", areaDIMLIGHT);
		engine->RegisterEnumValue("Area", "SETLIGHT", areaSETLIGHT);
		engine->RegisterEnumValue("Area", "LIMITXSCROLL", areaLIMITXSCROLL);
		engine->RegisterEnumValue("Area", "RESETLIGHT", areaRESETLIGHT);
		engine->RegisterEnumValue("Area", "WARPSECRET", areaWARPSECRET);
		engine->RegisterEnumValue("Area", "ECHO", areaECHO);
		engine->RegisterEnumValue("Area", "ACTIVATEBOSS", areaBOSSTRIGGER);
		engine->RegisterEnumValue("Area", "JAZZLEVELSTART", areaJAZZLEVELSTART);
		engine->RegisterEnumValue("Area", "JAZZSTART", areaJAZZLEVELSTART); // Private/deprecated
		engine->RegisterEnumValue("Area", "SPAZLEVELSTART", areaSPAZLEVELSTART);
		engine->RegisterEnumValue("Area", "SPAZSTART", areaSPAZLEVELSTART); // Private/deprecated
		engine->RegisterEnumValue("Area", "MPLEVELSTART", areaMPLEVELSTART);
		engine->RegisterEnumValue("Area", "MPSTART", areaMPLEVELSTART); // Private/deprecated
		engine->RegisterEnumValue("Area", "LORILEVELSTART", areaLORILEVELSTART);
		engine->RegisterEnumValue("Area", "LORISTART", areaLORILEVELSTART); // Private/deprecated
		engine->RegisterEnumValue("Area", "WARP", areaWARP);
		engine->RegisterEnumValue("Area", "WARPTARGET", areaWARPTARGET);
		engine->RegisterEnumValue("Area", "PATH", areaAREAID);
		engine->RegisterEnumValue("Area", "AREAID", areaAREAID); // Private/deprecated
		engine->RegisterEnumValue("Area", "NOFIREZONE", areaNOFIREZONE);
		engine->RegisterEnumValue("Area", "TRIGGERZONE", areaTRIGGERZONE);

		engine->RegisterEnumValue("Area", "SUCKERTUBE", aSUCKERTUBE);
		engine->RegisterEnumValue("Area", "TEXT", aTEXT);
		engine->RegisterEnumValue("Area", "WATERLEVEL", aWATERLEVEL);
		engine->RegisterEnumValue("Area", "MORPHFROG", aMORPHFROG);
		engine->RegisterEnumValue("Area", "WATERBLOCK", aWATERBLOCK);

		engine->SetDefaultNamespace("OBJECT");
		engine->RegisterEnumValue("Object", "BLASTERBULLET", aPLAYERBULLET1);
		engine->RegisterEnumValue("Object", "BOUNCERBULLET", aPLAYERBULLET2);
		engine->RegisterEnumValue("Object", "ICEBULLET", aPLAYERBULLET3);
		engine->RegisterEnumValue("Object", "SEEKERBULLET", aPLAYERBULLET4);
		engine->RegisterEnumValue("Object", "RFBULLET", aPLAYERBULLET5);
		engine->RegisterEnumValue("Object", "TOASTERBULLET", aPLAYERBULLET6);
		engine->RegisterEnumValue("Object", "FIREBALLBULLET", aPLAYERBULLET8);
		engine->RegisterEnumValue("Object", "ELECTROBULLET", aPLAYERBULLET9);
		engine->RegisterEnumValue("Object", "BLASTERBULLETPU", aPLAYERBULLETP1);
		engine->RegisterEnumValue("Object", "BOUNCERBULLETPU", aPLAYERBULLETP2);
		engine->RegisterEnumValue("Object", "ICEBULLETPU", aPLAYERBULLETP3);
		engine->RegisterEnumValue("Object", "SEEKERBULLETPU", aPLAYERBULLETP4);
		engine->RegisterEnumValue("Object", "RFBULLETPU", aPLAYERBULLETP5);
		engine->RegisterEnumValue("Object", "TOASTERBULLETPU", aPLAYERBULLETP6);
		engine->RegisterEnumValue("Object", "FIREBALLBULLETPU", aPLAYERBULLETP8);
		engine->RegisterEnumValue("Object", "ELECTROBULLETPU", aPLAYERBULLETP9);
		engine->RegisterEnumValue("Object", "FIRESHIELDBULLET", aPLAYERBULLETC1);
		engine->RegisterEnumValue("Object", "WATERSHIELDBULLET", aPLAYERBULLETC2);
		engine->RegisterEnumValue("Object", "BUBBLESHIELDBULLET", aPLAYERBULLETC2); // Private/deprecated
		engine->RegisterEnumValue("Object", "LIGHTNINGSHIELDBULLET", aPLAYERBULLETC3);
		engine->RegisterEnumValue("Object", "PLASMASHIELDBULLET", aPLAYERBULLETC3); // Private/deprecated
		engine->RegisterEnumValue("Object", "BULLET", aBULLET);
		engine->RegisterEnumValue("Object", "SMOKERING", aCATSMOKE);
		engine->RegisterEnumValue("Object", "SHARD", aSHARD);
		engine->RegisterEnumValue("Object", "EXPLOSION", aEXPLOSION);
		engine->RegisterEnumValue("Object", "BOUNCEONCE", aBOUNCEONCE);
		engine->RegisterEnumValue("Object", "FLICKERGEM", aREDGEMTEMP);
		engine->RegisterEnumValue("Object", "LASER", aPLAYERLASER);
		engine->RegisterEnumValue("Object", "UTERUSSPIKEBALL", aUTERUSEL);
		engine->RegisterEnumValue("Object", "BIRD", aBIRD);
		engine->RegisterEnumValue("Object", "BUBBLE", aBUBBLE);
		engine->RegisterEnumValue("Object", "ICEAMMO3", aGUN3AMMO3);
		engine->RegisterEnumValue("Object", "BOUNCERAMMO3", aGUN2AMMO3);
		engine->RegisterEnumValue("Object", "SEEKERAMMO3", aGUN4AMMO3);
		engine->RegisterEnumValue("Object", "RFAMMO3", aGUN5AMMO3);
		engine->RegisterEnumValue("Object", "TOASTERAMMO3", aGUN6AMMO3);
		engine->RegisterEnumValue("Object", "TNTAMMO3", aGUN7AMMO3);
		engine->RegisterEnumValue("Object", "GUN8AMMO3", aGUN8AMMO3);
		engine->RegisterEnumValue("Object", "GUN9AMMO3", aGUN9AMMO3);
		engine->RegisterEnumValue("Object", "TURTLESHELL", aTURTLESHELL);
		engine->RegisterEnumValue("Object", "SWINGINGVINE", aSWINGVINE);
		engine->RegisterEnumValue("Object", "BOMB", aBOMB);
		engine->RegisterEnumValue("Object", "SILVERCOIN", aSILVERCOIN);
		engine->RegisterEnumValue("Object", "GOLDCOIN", aGOLDCOIN);
		engine->RegisterEnumValue("Object", "GUNCRATE", aGUNCRATE);
		engine->RegisterEnumValue("Object", "CARROTCRATE", aCARROTCRATE);
		engine->RegisterEnumValue("Object", "ONEUPCRATE", a1UPCRATE);
		engine->RegisterEnumValue("Object", "GEMBARREL", aGEMBARREL);
		engine->RegisterEnumValue("Object", "CARROTBARREL", aCARROTBARREL);
		engine->RegisterEnumValue("Object", "ONEUPBARREL", a1UPBARREL);
		engine->RegisterEnumValue("Object", "BOMBCRATE", aBOMBCRATE);
		engine->RegisterEnumValue("Object", "ICEAMMO15", aGUN3AMMO15);
		engine->RegisterEnumValue("Object", "BOUNCERAMMO15", aGUN2AMMO15);
		engine->RegisterEnumValue("Object", "SEEKERAMMO15", aGUN4AMMO15);
		engine->RegisterEnumValue("Object", "RFAMMO15", aGUN5AMMO15);
		engine->RegisterEnumValue("Object", "TOASTERAMMO15", aGUN6AMMO15);
		engine->RegisterEnumValue("Object", "TNT", aTNT);
		engine->RegisterEnumValue("Object", "AIRBOARDGENERATOR", aAIRBOARDGENERATOR);
		engine->RegisterEnumValue("Object", "FROZENSPRING", aFROZENGREENSPRING);
		engine->RegisterEnumValue("Object", "FASTFIRE", aGUNFASTFIRE);
		engine->RegisterEnumValue("Object", "SPRINGCRATE", aSPRINGCRATE);
		engine->RegisterEnumValue("Object", "REDGEM", aREDGEM);
		engine->RegisterEnumValue("Object", "GREENGEM", aGREENGEM);
		engine->RegisterEnumValue("Object", "BLUEGEM", aBLUEGEM);
		engine->RegisterEnumValue("Object", "PURPLEGEM", aPURPLEGEM);
		engine->RegisterEnumValue("Object", "SUPERGEM", aSUPERREDGEM);
		engine->RegisterEnumValue("Object", "BIRDCAGE", aBIRDCAGE);
		engine->RegisterEnumValue("Object", "GUNBARREL", aGUNBARREL);
		engine->RegisterEnumValue("Object", "GEMCRATE", aGEMCRATE);
		engine->RegisterEnumValue("Object", "MORPH", aMORPHMONITOR);
		engine->RegisterEnumValue("Object", "CARROT", aENERGYUP);
		engine->RegisterEnumValue("Object", "FULLENERGY", aFULLENERGY);
		engine->RegisterEnumValue("Object", "FIRESHIELD", aFIRESHIELD);
		engine->RegisterEnumValue("Object", "WATERSHIELD", aWATERSHIELD);
		engine->RegisterEnumValue("Object", "BUBBLESHIELD", aWATERSHIELD); // Private/deprecated
		engine->RegisterEnumValue("Object", "LIGHTNINGSHIELD", aLIGHTSHIELD);
		engine->RegisterEnumValue("Object", "PLASMASHIELD", aLIGHTSHIELD); // Private/deprecated
		engine->RegisterEnumValue("Object", "FASTFEET", aFASTFEET);
		engine->RegisterEnumValue("Object", "ONEUP", aEXTRALIFE);
		engine->RegisterEnumValue("Object", "EXTRALIFE", aEXTRALIFE); // Private/deprecated
		engine->RegisterEnumValue("Object", "EXTRALIVE", aEXTRALIFE); // Private/deprecated
		engine->RegisterEnumValue("Object", "EOLPOST", aENDOFLEVELPOST);
		engine->RegisterEnumValue("Object", "SAVEPOST", aSAVEPOST);
		engine->RegisterEnumValue("Object", "CHECKPOINT", aSAVEPOST); // Private/deprecated
		engine->RegisterEnumValue("Object", "BONUSPOST", aBONUSLEVELPOST);
		engine->RegisterEnumValue("Object", "REDSPRING", aREDSPRING);
		engine->RegisterEnumValue("Object", "GREENSPRING", aGREENSPRING);
		engine->RegisterEnumValue("Object", "BLUESPRING", aBLUESPRING);
		engine->RegisterEnumValue("Object", "INVINCIBILITY", aINVINCIBILITY);
		engine->RegisterEnumValue("Object", "EXTRATIME", aEXTRATIME);
		engine->RegisterEnumValue("Object", "FREEZER", aFREEZER);
		engine->RegisterEnumValue("Object", "FREEZEENEMIES", aFREEZER); // Private/deprecated
		engine->RegisterEnumValue("Object", "HORREDSPRING", aHREDSPRING);
		engine->RegisterEnumValue("Object", "HORGREENSPRING", aHGREENSPRING);
		engine->RegisterEnumValue("Object", "HORBLUESPRING", aHBLUESPRING);
		engine->RegisterEnumValue("Object", "BIRDMORPH", aBIRDMORPHMONITOR);
		engine->RegisterEnumValue("Object", "TRIGGERCRATE", aTRIGGERCRATE);
		engine->RegisterEnumValue("Object", "FLYCARROT", aFLYCARROT);
		engine->RegisterEnumValue("Object", "RECTREDGEM", aRECTREDGEM);
		engine->RegisterEnumValue("Object", "RECTGREENGEM", aRECTGREENGEM);
		engine->RegisterEnumValue("Object", "RECTBLUEGEM", aRECTBLUEGEM);
		engine->RegisterEnumValue("Object", "TUFTURT", aTUFTURT);
		engine->RegisterEnumValue("Object", "TUFBOSS", aTUFBOSS);
		engine->RegisterEnumValue("Object", "LABRAT", aLABRAT);
		engine->RegisterEnumValue("Object", "DRAGON", aDRAGON);
		engine->RegisterEnumValue("Object", "LIZARD", aLIZARD);
		engine->RegisterEnumValue("Object", "BEE", aBUMBEE);
		engine->RegisterEnumValue("Object", "BUMBEE", aBUMBEE); // Private/deprecated
		engine->RegisterEnumValue("Object", "RAPIER", aRAPIER);
		engine->RegisterEnumValue("Object", "SPARK", aSPARK);
		engine->RegisterEnumValue("Object", "BAT", aBAT);
		engine->RegisterEnumValue("Object", "SUCKER", aSUCKER);
		engine->RegisterEnumValue("Object", "CATERPILLAR", aCATERPILLAR);
		engine->RegisterEnumValue("Object", "CHESHIRE1", aCHESHIRE1);
		engine->RegisterEnumValue("Object", "CHESHIRE2", aCHESHIRE2);
		engine->RegisterEnumValue("Object", "HATTER", aHATTER);
		engine->RegisterEnumValue("Object", "BILSY", aBILSYBOSS);
		engine->RegisterEnumValue("Object", "SKELETON", aSKELETON);
		engine->RegisterEnumValue("Object", "DOGGYDOGG", aDOGGYDOGG);
		engine->RegisterEnumValue("Object", "NORMTURTLE", aNORMTURTLE);
		engine->RegisterEnumValue("Object", "HELMUT", aHELMUT);
		engine->RegisterEnumValue("Object", "DEMON", aDEMON);
		engine->RegisterEnumValue("Object", "DRAGONFLY", aDRAGONFLY);
		engine->RegisterEnumValue("Object", "MONKEY", aMONKEY);
		engine->RegisterEnumValue("Object", "FATCHICK", aFATCHK);
		engine->RegisterEnumValue("Object", "FENCER", aFENCER);
		engine->RegisterEnumValue("Object", "FISH", aFISH);
		engine->RegisterEnumValue("Object", "MOTH", aMOTH);
		engine->RegisterEnumValue("Object", "STEAM", aSTEAM);
		engine->RegisterEnumValue("Object", "ROTATINGROCK", aROCK);
		engine->RegisterEnumValue("Object", "BLASTERPOWERUP", aGUN1POWER);
		engine->RegisterEnumValue("Object", "BOUNCERPOWERUP", aGUN2POWER);
		engine->RegisterEnumValue("Object", "ICEPOWERUP", aGUN3POWER);
		engine->RegisterEnumValue("Object", "SEEKERPOWERUP", aGUN4POWER);
		engine->RegisterEnumValue("Object", "RFPOWERUP", aGUN5POWER);
		engine->RegisterEnumValue("Object", "TOASTERPOWERUP", aGUN6POWER);
		engine->RegisterEnumValue("Object", "LEFTPADDLE", aPINLEFTPADDLE);
		engine->RegisterEnumValue("Object", "RIGHTPADDLE", aPINRIGHTPADDLE);
		engine->RegisterEnumValue("Object", "FIVEHUNDREDBUMP", aPIN500BUMP);
		engine->RegisterEnumValue("Object", "CARROTBUMP", aPINCARROTBUMP);
		engine->RegisterEnumValue("Object", "APPLE", aAPPLE);
		engine->RegisterEnumValue("Object", "BANANA", aBANANA);
		engine->RegisterEnumValue("Object", "CHERRY", aCHERRY);
		engine->RegisterEnumValue("Object", "ORANGE", aORANGE);
		engine->RegisterEnumValue("Object", "PEAR", aPEAR);
		engine->RegisterEnumValue("Object", "PRETZEL", aPRETZEL);
		engine->RegisterEnumValue("Object", "STRAWBERRY", aSTRAWBERRY);
		engine->RegisterEnumValue("Object", "STEADYLIGHT", aSTEADYLIGHT);
		engine->RegisterEnumValue("Object", "PULZELIGHT", aPULZELIGHT);
		engine->RegisterEnumValue("Object", "PULSELIGHT", aPULZELIGHT); // Private/deprecated
		engine->RegisterEnumValue("Object", "FLICKERLIGHT", aFLICKERLIGHT);
		engine->RegisterEnumValue("Object", "QUEEN", aQUEENBOSS);
		engine->RegisterEnumValue("Object", "FLOATSUCKER", aFLOATSUCKER);
		engine->RegisterEnumValue("Object", "BRIDGE", aBRIDGE);
		engine->RegisterEnumValue("Object", "LEMON", aLEMON);
		engine->RegisterEnumValue("Object", "LIME", aLIME);
		engine->RegisterEnumValue("Object", "THING", aTHING);
		engine->RegisterEnumValue("Object", "WATERMELON", aWMELON);
		engine->RegisterEnumValue("Object", "PEACH", aPEACH);
		engine->RegisterEnumValue("Object", "GRAPES", aGRAPES);
		engine->RegisterEnumValue("Object", "LETTUCE", aLETTUCE);
		engine->RegisterEnumValue("Object", "EGGPLANT", aEGGPLANT);
		engine->RegisterEnumValue("Object", "CUCUMB", aCUCUMB);
		engine->RegisterEnumValue("Object", "CUCUMBER", aCUCUMB); // Private/deprecated
		engine->RegisterEnumValue("Object", "COKE", aCOKE);
		engine->RegisterEnumValue("Object", "SOFTDRINK", aCOKE); // Private/deprecated
		engine->RegisterEnumValue("Object", "PEPSI", aPEPSI);
		engine->RegisterEnumValue("Object", "SODAPOP", aCOKE); // Private/deprecated
		engine->RegisterEnumValue("Object", "MILK", aMILK);
		engine->RegisterEnumValue("Object", "PIE", aPIE);
		engine->RegisterEnumValue("Object", "CAKE", aCAKE);
		engine->RegisterEnumValue("Object", "DONUT", aDONUT);
		engine->RegisterEnumValue("Object", "CUPCAKE", aCUPCAKE);
		engine->RegisterEnumValue("Object", "CHIPS", aCHIPS);
		engine->RegisterEnumValue("Object", "CANDY", aCANDY1);
		engine->RegisterEnumValue("Object", "CHOCBAR", aCHOCBAR);
		engine->RegisterEnumValue("Object", "aCHOCOLATEBAR", aCHOCBAR); // Private/deprecated
		engine->RegisterEnumValue("Object", "ICECREAM", aICECREAM);
		engine->RegisterEnumValue("Object", "BURGER", aBURGER);
		engine->RegisterEnumValue("Object", "PIZZA", aPIZZA);
		engine->RegisterEnumValue("Object", "FRIES", aFRIES);
		engine->RegisterEnumValue("Object", "CHICKENLEG", aCHICKLEG);
		engine->RegisterEnumValue("Object", "SANDWICH", aSANDWICH);
		engine->RegisterEnumValue("Object", "TACO", aTACOBELL);
		engine->RegisterEnumValue("Object", "WEENIE", aWEENIE);
		engine->RegisterEnumValue("Object", "HAM", aHAM);
		engine->RegisterEnumValue("Object", "CHEESE", aCHEESE);
		engine->RegisterEnumValue("Object", "FLOATLIZARD", aFLOATLIZARD);
		engine->RegisterEnumValue("Object", "STANDMONKEY", aSTANDMONKEY);
		engine->RegisterEnumValue("Object", "DESTRUCTSCENERY", aDESTRUCTSCENERY);
		engine->RegisterEnumValue("Object", "DESTRUCTSCENERYBOMB", aDESTRUCTSCENERYBOMB);
		engine->RegisterEnumValue("Object", "TNTDESTRUCTSCENERY", aDESTRUCTSCENERYBOMB); // Private/deprecated
		engine->RegisterEnumValue("Object", "COLLAPSESCENERY", aCOLLAPSESCENERY);
		engine->RegisterEnumValue("Object", "STOMPSCENERY", aSTOMPSCENERY);
		engine->RegisterEnumValue("Object", "GEMSTOMP", aGEMSTOMP);
		engine->RegisterEnumValue("Object", "RAVEN", aRAVEN);
		engine->RegisterEnumValue("Object", "TUBETURTLE", aTUBETURTLE);
		engine->RegisterEnumValue("Object", "GEMRING", aGEMRING);
		engine->RegisterEnumValue("Object", "SMALLTREE", aROTSMALLTREE);
		engine->RegisterEnumValue("Object", "AMBIENTSOUND", aAMBIENTSOUND);
		engine->RegisterEnumValue("Object", "UTERUS", aUTERUS);
		engine->RegisterEnumValue("Object", "CRAB", aCRAB);
		engine->RegisterEnumValue("Object", "WITCH", aWITCH);
		engine->RegisterEnumValue("Object", "ROCKETTURTLE", aROCKTURT);
		engine->RegisterEnumValue("Object", "BUBBA", aBUBBA);
		engine->RegisterEnumValue("Object", "DEVILDEVAN", aDEVILDEVAN);
		engine->RegisterEnumValue("Object", "DEVANROBOT", aDEVANROBOT);
		engine->RegisterEnumValue("Object", "ROBOT", aROBOT);
		engine->RegisterEnumValue("Object", "CARROTUSPOLE", aCARROTUSPOLE);
		engine->RegisterEnumValue("Object", "PSYCHPOLE", aPSYCHPOLE);
		engine->RegisterEnumValue("Object", "DIAMONDUSPOLE", aDIAMONDUSPOLE);
		engine->RegisterEnumValue("Object", "FRUITPLATFORM", aFRUITPLATFORM);
		engine->RegisterEnumValue("Object", "BOLLPLATFORM", aBOLLPLATFORM);
		engine->RegisterEnumValue("Object", "GRASSPLATFORM", aGRASSPLATFORM);
		engine->RegisterEnumValue("Object", "PINKPLATFORM", aPINKPLATFORM);
		engine->RegisterEnumValue("Object", "SONICPLATFORM", aSONICPLATFORM);
		engine->RegisterEnumValue("Object", "SPIKEPLATFORM", aSPIKEPLATFORM);
		engine->RegisterEnumValue("Object", "SPIKEBOLL", aSPIKEBOLL);
		engine->RegisterEnumValue("Object", "GENERATOR", aGENERATOR);
		engine->RegisterEnumValue("Object", "EVA", aEVA);
		engine->RegisterEnumValue("Object", "BUBBLER", aBUBBLER);
		engine->RegisterEnumValue("Object", "TNTPOWERUP", aTNTPOWER);
		engine->RegisterEnumValue("Object", "GUN8POWERUP", aGUN8POWER);
		engine->RegisterEnumValue("Object", "GUN9POWERUP", aGUN9POWER);
		engine->RegisterEnumValue("Object", "SPIKEBOLL3D", aSPIKEBOLL3D);
		engine->RegisterEnumValue("Object", "SPRINGCORD", aSPRINGCORD);
		engine->RegisterEnumValue("Object", "BEES", aBEES);
		engine->RegisterEnumValue("Object", "COPTER", aCOPTER);
		engine->RegisterEnumValue("Object", "LASERSHIELD", aLASERSHIELD);
		engine->RegisterEnumValue("Object", "STOPWATCH", aSTOPWATCH);
		engine->RegisterEnumValue("Object", "JUNGLEPOLE", aJUNGLEPOLE);
		engine->RegisterEnumValue("Object", "WARP", areaWARP);
		engine->RegisterEnumValue("Object", "BIGROCK", aBIGROCK);
		engine->RegisterEnumValue("Object", "BIGBOX", aBIGBOX);
		engine->RegisterEnumValue("Object", "TRIGGERSCENERY", aTRIGGERSCENERY);
		engine->RegisterEnumValue("Object", "BOLLY", aSONICBOSS);
		engine->RegisterEnumValue("Object", "BUTTERFLY", aBUTTERFLY);
		engine->RegisterEnumValue("Object", "BEEBOY", aBEEBOY);
		engine->RegisterEnumValue("Object", "SNOW", aSNOW);
		engine->RegisterEnumValue("Object", "TWEEDLEBOSS", aTWEEDLEBOSS);
		engine->RegisterEnumValue("Object", "AIRBOARD", aAIRBOARD);
		engine->RegisterEnumValue("Object", "CTFBASE", aFLAG);
		engine->RegisterEnumValue("Object", "XMASNORMTURTLE", aXNORMTURTLE);
		engine->RegisterEnumValue("Object", "XMASLIZARD", aXLIZARD);
		engine->RegisterEnumValue("Object", "XMASFLOATLIZARD", aXFLOATLIZARD);
		engine->RegisterEnumValue("Object", "XMASBILSY", aXBILSYBOSS);
		engine->RegisterEnumValue("Object", "CAT", aZCAT);
		engine->RegisterEnumValue("Object", "PACMANGHOST", aZGHOST);

		engine->SetDefaultNamespace("BEHAVIOR");
		engine->RegisterEnumValue("Behavior", "DEFAULT", -1);
		engine->RegisterEnumValue("Behavior", "INACTIVE", aUNKNOWN);
		engine->RegisterEnumValue("Behavior", "BLASTERBULLET", aPLAYERBULLET1);
		engine->RegisterEnumValue("Behavior", "BOUNCERBULLET", aPLAYERBULLET2);
		engine->RegisterEnumValue("Behavior", "ICEBULLET", aPLAYERBULLET3);
		engine->RegisterEnumValue("Behavior", "SEEKERBULLET", aPLAYERBULLET4);
		engine->RegisterEnumValue("Behavior", "RFBULLET", aPLAYERBULLET5);
		engine->RegisterEnumValue("Behavior", "TOASTERBULLET", aPLAYERBULLET6);
		engine->RegisterEnumValue("Behavior", "FIREBALLBULLET", aPLAYERBULLET8);
		engine->RegisterEnumValue("Behavior", "ELECTROBULLET", aPLAYERBULLET9);
		engine->RegisterEnumValue("Behavior", "BLASTERBULLETPU", aPLAYERBULLETP1);
		engine->RegisterEnumValue("Behavior", "BOUNCERBULLETPU", aPLAYERBULLETP2);
		engine->RegisterEnumValue("Behavior", "ICEBULLETPU", aPLAYERBULLETP3);
		engine->RegisterEnumValue("Behavior", "SEEKERBULLETPU", aPLAYERBULLETP4);
		engine->RegisterEnumValue("Behavior", "RFBULLETPU", aPLAYERBULLETP5);
		engine->RegisterEnumValue("Behavior", "TOASTERBULLETPU", aPLAYERBULLETP6);
		engine->RegisterEnumValue("Behavior", "FIREBALLBULLETPU", aPLAYERBULLETP8);
		engine->RegisterEnumValue("Behavior", "ELECTROBULLETPU", aPLAYERBULLETP9);
		engine->RegisterEnumValue("Behavior", "FIRESHIELDBULLET", aPLAYERBULLETC1);
		engine->RegisterEnumValue("Behavior", "WATERSHIELDBULLET", aPLAYERBULLETC2);
		engine->RegisterEnumValue("Behavior", "BUBBLESHIELDBULLET", aPLAYERBULLETC2); // Private/deprecated
		engine->RegisterEnumValue("Behavior", "LIGHTNINGSHIELDBULLET", aPLAYERBULLETC3);
		engine->RegisterEnumValue("Behavior", "PLASMASHIELDBULLET", aPLAYERBULLETC3); // Private/deprecated
		engine->RegisterEnumValue("Behavior", "BULLET", aBULLET);
		engine->RegisterEnumValue("Behavior", "SMOKERING", aCATSMOKE);
		engine->RegisterEnumValue("Behavior", "SHARD", aSHARD);
		engine->RegisterEnumValue("Behavior", "EXPLOSION", aEXPLOSION);
		engine->RegisterEnumValue("Behavior", "BOUNCEONCE", aBOUNCEONCE);
		engine->RegisterEnumValue("Behavior", "FLICKERGEM", aREDGEMTEMP);
		engine->RegisterEnumValue("Behavior", "LASER", aPLAYERLASER);
		engine->RegisterEnumValue("Behavior", "UTERUSSPIKEBALL", aUTERUSEL);
		engine->RegisterEnumValue("Behavior", "BIRD", aBIRD);
		engine->RegisterEnumValue("Behavior", "BUBBLE", aBUBBLE);
		engine->RegisterEnumValue("Behavior", "ICEAMMO3", aGUN3AMMO3);
		engine->RegisterEnumValue("Behavior", "BOUNCERAMMO3", aGUN2AMMO3);
		engine->RegisterEnumValue("Behavior", "SEEKERAMMO3", aGUN4AMMO3);
		engine->RegisterEnumValue("Behavior", "RFAMMO3", aGUN5AMMO3);
		engine->RegisterEnumValue("Behavior", "TOASTERAMMO3", aGUN6AMMO3);
		engine->RegisterEnumValue("Behavior", "TNTAMMO3", aGUN7AMMO3);
		engine->RegisterEnumValue("Behavior", "GUN8AMMO3", aGUN8AMMO3);
		engine->RegisterEnumValue("Behavior", "GUN9AMMO3", aGUN9AMMO3);
		engine->RegisterEnumValue("Behavior", "TURTLESHELL", aTURTLESHELL);
		engine->RegisterEnumValue("Behavior", "SWINGINGVINE", aSWINGVINE);
		engine->RegisterEnumValue("Behavior", "BOMB", aBOMB);
		engine->RegisterEnumValue("Behavior", "SILVERCOIN", aSILVERCOIN);
		engine->RegisterEnumValue("Behavior", "GOLDCOIN", aGOLDCOIN);
		engine->RegisterEnumValue("Behavior", "GUNCRATE", aGUNCRATE);
		engine->RegisterEnumValue("Behavior", "CARROTCRATE", aCARROTCRATE);
		engine->RegisterEnumValue("Behavior", "ONEUPCRATE", a1UPCRATE);
		engine->RegisterEnumValue("Behavior", "GEMBARREL", aGEMBARREL);
		engine->RegisterEnumValue("Behavior", "CARROTBARREL", aCARROTBARREL);
		engine->RegisterEnumValue("Behavior", "ONEUPBARREL", a1UPBARREL);
		engine->RegisterEnumValue("Behavior", "BOMBCRATE", aBOMBCRATE);
		engine->RegisterEnumValue("Behavior", "ICEAMMO15", aGUN3AMMO15);
		engine->RegisterEnumValue("Behavior", "BOUNCERAMMO15", aGUN2AMMO15);
		engine->RegisterEnumValue("Behavior", "SEEKERAMMO15", aGUN4AMMO15);
		engine->RegisterEnumValue("Behavior", "RFAMMO15", aGUN5AMMO15);
		engine->RegisterEnumValue("Behavior", "TOASTERAMMO15", aGUN6AMMO15);
		engine->RegisterEnumValue("Behavior", "TNT", aTNT);
		engine->RegisterEnumValue("Behavior", "AIRBOARDGENERATOR", aAIRBOARDGENERATOR);
		engine->RegisterEnumValue("Behavior", "FROZENSPRING", aFROZENGREENSPRING);
		engine->RegisterEnumValue("Behavior", "FASTFIRE", aGUNFASTFIRE);
		engine->RegisterEnumValue("Behavior", "SPRINGCRATE", aSPRINGCRATE);
		engine->RegisterEnumValue("Behavior", "REDGEM", aREDGEM);
		engine->RegisterEnumValue("Behavior", "GREENGEM", aGREENGEM);
		engine->RegisterEnumValue("Behavior", "BLUEGEM", aBLUEGEM);
		engine->RegisterEnumValue("Behavior", "PURPLEGEM", aPURPLEGEM);
		engine->RegisterEnumValue("Behavior", "SUPERGEM", aSUPERREDGEM);
		engine->RegisterEnumValue("Behavior", "BIRDCAGE", aBIRDCAGE);
		engine->RegisterEnumValue("Behavior", "GUNBARREL", aGUNBARREL);
		engine->RegisterEnumValue("Behavior", "GEMCRATE", aGEMCRATE);
		engine->RegisterEnumValue("Behavior", "MORPH", aMORPHMONITOR);
		engine->RegisterEnumValue("Behavior", "CARROT", aENERGYUP);
		engine->RegisterEnumValue("Behavior", "FULLENERGY", aFULLENERGY);
		engine->RegisterEnumValue("Behavior", "FIRESHIELD", aFIRESHIELD);
		engine->RegisterEnumValue("Behavior", "WATERSHIELD", aWATERSHIELD);
		engine->RegisterEnumValue("Behavior", "BUBBLESHIELD", aWATERSHIELD); // Private/deprecated
		engine->RegisterEnumValue("Behavior", "LIGHTNINGSHIELD", aLIGHTSHIELD);
		engine->RegisterEnumValue("Behavior", "PLASMASHIELD", aLIGHTSHIELD); // Private/deprecated
		engine->RegisterEnumValue("Behavior", "FASTFEET", aFASTFEET);
		engine->RegisterEnumValue("Behavior", "ONEUP", aEXTRALIFE);
		engine->RegisterEnumValue("Behavior", "EXTRALIFE", aEXTRALIFE); // Private/deprecated
		engine->RegisterEnumValue("Behavior", "EXTRALIVE", aEXTRALIFE); // Private/deprecated
		engine->RegisterEnumValue("Behavior", "EOLPOST", aENDOFLEVELPOST);
		engine->RegisterEnumValue("Behavior", "SAVEPOST", aSAVEPOST);
		engine->RegisterEnumValue("Behavior", "CHECKPOINT", aSAVEPOST); // Private/deprecated
		engine->RegisterEnumValue("Behavior", "BONUSPOST", aBONUSLEVELPOST);
		engine->RegisterEnumValue("Behavior", "REDSPRING", aREDSPRING);
		engine->RegisterEnumValue("Behavior", "GREENSPRING", aGREENSPRING);
		engine->RegisterEnumValue("Behavior", "BLUESPRING", aBLUESPRING);
		engine->RegisterEnumValue("Behavior", "INVINCIBILITY", aINVINCIBILITY);
		engine->RegisterEnumValue("Behavior", "EXTRATIME", aEXTRATIME);
		engine->RegisterEnumValue("Behavior", "FREEZER", aFREEZER);
		engine->RegisterEnumValue("Behavior", "FREEZEENEMIES", aFREEZER); // Private/deprecated
		engine->RegisterEnumValue("Behavior", "HORREDSPRING", aHREDSPRING);
		engine->RegisterEnumValue("Behavior", "HORGREENSPRING", aHGREENSPRING);
		engine->RegisterEnumValue("Behavior", "HORBLUESPRING", aHBLUESPRING);
		engine->RegisterEnumValue("Behavior", "BIRDMORPH", aBIRDMORPHMONITOR);
		engine->RegisterEnumValue("Behavior", "TRIGGERCRATE", aTRIGGERCRATE);
		engine->RegisterEnumValue("Behavior", "FLYCARROT", aFLYCARROT);
		engine->RegisterEnumValue("Behavior", "RECTREDGEM", aRECTREDGEM);
		engine->RegisterEnumValue("Behavior", "RECTGREENGEM", aRECTGREENGEM);
		engine->RegisterEnumValue("Behavior", "RECTBLUEGEM", aRECTBLUEGEM);
		engine->RegisterEnumValue("Behavior", "TUFTURT", aTUFTURT);
		engine->RegisterEnumValue("Behavior", "TUFBOSS", aTUFBOSS);
		engine->RegisterEnumValue("Behavior", "LABRAT", aLABRAT);
		engine->RegisterEnumValue("Behavior", "DRAGON", aDRAGON);
		engine->RegisterEnumValue("Behavior", "LIZARD", aLIZARD);
		engine->RegisterEnumValue("Behavior", "BEE", aBUMBEE);
		engine->RegisterEnumValue("Behavior", "BUMBEE", aBUMBEE); // Private/deprecated
		engine->RegisterEnumValue("Behavior", "RAPIER", aRAPIER);
		engine->RegisterEnumValue("Behavior", "SPARK", aSPARK);
		engine->RegisterEnumValue("Behavior", "BAT", aBAT);
		engine->RegisterEnumValue("Behavior", "SUCKER", aSUCKER);
		engine->RegisterEnumValue("Behavior", "CATERPILLAR", aCATERPILLAR);
		engine->RegisterEnumValue("Behavior", "CHESHIRE1", aCHESHIRE1);
		engine->RegisterEnumValue("Behavior", "CHESHIRE2", aCHESHIRE2);
		engine->RegisterEnumValue("Behavior", "HATTER", aHATTER);
		engine->RegisterEnumValue("Behavior", "BILSY", aBILSYBOSS);
		engine->RegisterEnumValue("Behavior", "SKELETON", aSKELETON);
		engine->RegisterEnumValue("Behavior", "DOGGYDOGG", aDOGGYDOGG);
		engine->RegisterEnumValue("Behavior", "NORMTURTLE", aNORMTURTLE);
		engine->RegisterEnumValue("Behavior", "HELMUT", aHELMUT);
		engine->RegisterEnumValue("Behavior", "DEMON", aDEMON);
		engine->RegisterEnumValue("Behavior", "DRAGONFLY", aDRAGONFLY);
		engine->RegisterEnumValue("Behavior", "MONKEY", aMONKEY);
		engine->RegisterEnumValue("Behavior", "FATCHICK", aFATCHK);
		engine->RegisterEnumValue("Behavior", "FENCER", aFENCER);
		engine->RegisterEnumValue("Behavior", "FISH", aFISH);
		engine->RegisterEnumValue("Behavior", "MOTH", aMOTH);
		engine->RegisterEnumValue("Behavior", "STEAM", aSTEAM);
		engine->RegisterEnumValue("Behavior", "ROTATINGROCK", aROCK);
		engine->RegisterEnumValue("Behavior", "BLASTERPOWERUP", aGUN1POWER);
		engine->RegisterEnumValue("Behavior", "BOUNCERPOWERUP", aGUN2POWER);
		engine->RegisterEnumValue("Behavior", "ICEPOWERUP", aGUN3POWER);
		engine->RegisterEnumValue("Behavior", "SEEKERPOWERUP", aGUN4POWER);
		engine->RegisterEnumValue("Behavior", "RFPOWERUP", aGUN5POWER);
		engine->RegisterEnumValue("Behavior", "TOASTERPOWERUP", aGUN6POWER);
		engine->RegisterEnumValue("Behavior", "LEFTPADDLE", aPINLEFTPADDLE);
		engine->RegisterEnumValue("Behavior", "RIGHTPADDLE", aPINRIGHTPADDLE);
		engine->RegisterEnumValue("Behavior", "FIVEHUNDREDBUMP", aPIN500BUMP);
		engine->RegisterEnumValue("Behavior", "CARROTBUMP", aPINCARROTBUMP);
		engine->RegisterEnumValue("Behavior", "APPLE", aAPPLE);
		engine->RegisterEnumValue("Behavior", "BANANA", aBANANA);
		engine->RegisterEnumValue("Behavior", "CHERRY", aCHERRY);
		engine->RegisterEnumValue("Behavior", "ORANGE", aORANGE);
		engine->RegisterEnumValue("Behavior", "PEAR", aPEAR);
		engine->RegisterEnumValue("Behavior", "PRETZEL", aPRETZEL);
		engine->RegisterEnumValue("Behavior", "STRAWBERRY", aSTRAWBERRY);
		engine->RegisterEnumValue("Behavior", "STEADYLIGHT", aSTEADYLIGHT);
		engine->RegisterEnumValue("Behavior", "PULZELIGHT", aPULZELIGHT);
		engine->RegisterEnumValue("Behavior", "PULSELIGHT", aPULZELIGHT); // Private/deprecated
		engine->RegisterEnumValue("Behavior", "FLICKERLIGHT", aFLICKERLIGHT);
		engine->RegisterEnumValue("Behavior", "QUEEN", aQUEENBOSS);
		engine->RegisterEnumValue("Behavior", "FLOATSUCKER", aFLOATSUCKER);
		engine->RegisterEnumValue("Behavior", "BRIDGE", aBRIDGE);
		engine->RegisterEnumValue("Behavior", "LEMON", aLEMON);
		engine->RegisterEnumValue("Behavior", "LIME", aLIME);
		engine->RegisterEnumValue("Behavior", "THING", aTHING);
		engine->RegisterEnumValue("Behavior", "WATERMELON", aWMELON);
		engine->RegisterEnumValue("Behavior", "PEACH", aPEACH);
		engine->RegisterEnumValue("Behavior", "GRAPES", aGRAPES);
		engine->RegisterEnumValue("Behavior", "LETTUCE", aLETTUCE);
		engine->RegisterEnumValue("Behavior", "EGGPLANT", aEGGPLANT);
		engine->RegisterEnumValue("Behavior", "CUCUMB", aCUCUMB);
		engine->RegisterEnumValue("Behavior", "CUCUMBER", aCUCUMB); // Private/deprecated
		engine->RegisterEnumValue("Behavior", "COKE", aCOKE);
		engine->RegisterEnumValue("Behavior", "SOFTDRINK", aCOKE); // Private/deprecated
		engine->RegisterEnumValue("Behavior", "PEPSI", aPEPSI);
		engine->RegisterEnumValue("Behavior", "SODAPOP", aCOKE); // Private/deprecated
		engine->RegisterEnumValue("Behavior", "MILK", aMILK);
		engine->RegisterEnumValue("Behavior", "PIE", aPIE);
		engine->RegisterEnumValue("Behavior", "CAKE", aCAKE);
		engine->RegisterEnumValue("Behavior", "DONUT", aDONUT);
		engine->RegisterEnumValue("Behavior", "CUPCAKE", aCUPCAKE);
		engine->RegisterEnumValue("Behavior", "CHIPS", aCHIPS);
		engine->RegisterEnumValue("Behavior", "CANDY", aCANDY1);
		engine->RegisterEnumValue("Behavior", "CHOCBAR", aCHOCBAR);
		engine->RegisterEnumValue("Behavior", "aCHOCOLATEBAR", aCHOCBAR); // Private/deprecated
		engine->RegisterEnumValue("Behavior", "ICECREAM", aICECREAM);
		engine->RegisterEnumValue("Behavior", "BURGER", aBURGER);
		engine->RegisterEnumValue("Behavior", "PIZZA", aPIZZA);
		engine->RegisterEnumValue("Behavior", "FRIES", aFRIES);
		engine->RegisterEnumValue("Behavior", "CHICKENLEG", aCHICKLEG);
		engine->RegisterEnumValue("Behavior", "SANDWICH", aSANDWICH);
		engine->RegisterEnumValue("Behavior", "TACO", aTACOBELL);
		engine->RegisterEnumValue("Behavior", "WEENIE", aWEENIE);
		engine->RegisterEnumValue("Behavior", "HAM", aHAM);
		engine->RegisterEnumValue("Behavior", "CHEESE", aCHEESE);
		engine->RegisterEnumValue("Behavior", "FLOATLIZARD", aFLOATLIZARD);
		engine->RegisterEnumValue("Behavior", "STANDMONKEY", aSTANDMONKEY);
		engine->RegisterEnumValue("Behavior", "DESTRUCTSCENERY", aDESTRUCTSCENERY);
		engine->RegisterEnumValue("Behavior", "DESTRUCTSCENERYBOMB", aDESTRUCTSCENERYBOMB);
		engine->RegisterEnumValue("Behavior", "TNTDESTRUCTSCENERY", aDESTRUCTSCENERYBOMB); // Private/deprecated
		engine->RegisterEnumValue("Behavior", "COLLAPSESCENERY", aCOLLAPSESCENERY);
		engine->RegisterEnumValue("Behavior", "STOMPSCENERY", aSTOMPSCENERY);
		engine->RegisterEnumValue("Behavior", "GEMSTOMP", aGEMSTOMP);
		engine->RegisterEnumValue("Behavior", "RAVEN", aRAVEN);
		engine->RegisterEnumValue("Behavior", "TUBETURTLE", aTUBETURTLE);
		engine->RegisterEnumValue("Behavior", "GEMRING", aGEMRING);
		engine->RegisterEnumValue("Behavior", "SMALLTREE", aROTSMALLTREE);
		engine->RegisterEnumValue("Behavior", "AMBIENTSOUND", aAMBIENTSOUND);
		engine->RegisterEnumValue("Behavior", "UTERUS", aUTERUS);
		engine->RegisterEnumValue("Behavior", "CRAB", aCRAB);
		engine->RegisterEnumValue("Behavior", "WITCH", aWITCH);
		engine->RegisterEnumValue("Behavior", "ROCKETTURTLE", aROCKTURT);
		engine->RegisterEnumValue("Behavior", "BUBBA", aBUBBA);
		engine->RegisterEnumValue("Behavior", "DEVILDEVAN", aDEVILDEVAN);
		engine->RegisterEnumValue("Behavior", "DEVANROBOT", aDEVANROBOT);
		engine->RegisterEnumValue("Behavior", "ROBOT", aROBOT);
		engine->RegisterEnumValue("Behavior", "CARROTUSPOLE", aCARROTUSPOLE);
		engine->RegisterEnumValue("Behavior", "PSYCHPOLE", aPSYCHPOLE);
		engine->RegisterEnumValue("Behavior", "DIAMONDUSPOLE", aDIAMONDUSPOLE);
		engine->RegisterEnumValue("Behavior", "FRUITPLATFORM", aFRUITPLATFORM);
		engine->RegisterEnumValue("Behavior", "BOLLPLATFORM", aBOLLPLATFORM);
		engine->RegisterEnumValue("Behavior", "GRASSPLATFORM", aGRASSPLATFORM);
		engine->RegisterEnumValue("Behavior", "PINKPLATFORM", aPINKPLATFORM);
		engine->RegisterEnumValue("Behavior", "SONICPLATFORM", aSONICPLATFORM);
		engine->RegisterEnumValue("Behavior", "SPIKEPLATFORM", aSPIKEPLATFORM);
		engine->RegisterEnumValue("Behavior", "SPIKEBOLL", aSPIKEBOLL);
		engine->RegisterEnumValue("Behavior", "GENERATOR", aGENERATOR);
		engine->RegisterEnumValue("Behavior", "EVA", aEVA);
		engine->RegisterEnumValue("Behavior", "BUBBLER", aBUBBLER);
		engine->RegisterEnumValue("Behavior", "TNTPOWERUP", aTNTPOWER);
		engine->RegisterEnumValue("Behavior", "GUN8POWERUP", aGUN8POWER);
		engine->RegisterEnumValue("Behavior", "GUN9POWERUP", aGUN9POWER);
		engine->RegisterEnumValue("Behavior", "SPIKEBOLL3D", aSPIKEBOLL3D);
		engine->RegisterEnumValue("Behavior", "SPRINGCORD", aSPRINGCORD);
		engine->RegisterEnumValue("Behavior", "BEES", aBEES);
		engine->RegisterEnumValue("Behavior", "COPTER", aCOPTER);
		engine->RegisterEnumValue("Behavior", "LASERSHIELD", aLASERSHIELD);
		engine->RegisterEnumValue("Behavior", "STOPWATCH", aSTOPWATCH);
		engine->RegisterEnumValue("Behavior", "JUNGLEPOLE", aJUNGLEPOLE);
		engine->RegisterEnumValue("Behavior", "WARP", areaWARP);
		engine->RegisterEnumValue("Behavior", "BIGROCK", aBIGROCK);
		engine->RegisterEnumValue("Behavior", "BIGBOX", aBIGBOX);
		engine->RegisterEnumValue("Behavior", "TRIGGERSCENERY", aTRIGGERSCENERY);
		engine->RegisterEnumValue("Behavior", "BOLLY", aSONICBOSS);
		engine->RegisterEnumValue("Behavior", "BUTTERFLY", aBUTTERFLY);
		engine->RegisterEnumValue("Behavior", "BEEBOY", aBEEBOY);
		engine->RegisterEnumValue("Behavior", "SNOW", aSNOW);
		engine->RegisterEnumValue("Behavior", "TWEEDLEBOSS", aTWEEDLEBOSS);
		engine->RegisterEnumValue("Behavior", "AIRBOARD", aAIRBOARD);
		engine->RegisterEnumValue("Behavior", "CTFBASE", aFLAG);
		engine->RegisterEnumValue("Behavior", "XMASNORMTURTLE", aXNORMTURTLE);
		engine->RegisterEnumValue("Behavior", "XMASLIZARD", aXLIZARD);
		engine->RegisterEnumValue("Behavior", "XMASFLOATLIZARD", aXFLOATLIZARD);
		engine->RegisterEnumValue("Behavior", "XMASBILSY", aXBILSYBOSS);
		engine->RegisterEnumValue("Behavior", "CAT", aZCAT);
		engine->RegisterEnumValue("Behavior", "PACMANGHOST", aZGHOST);

		engine->RegisterEnumValue("Behavior", "WALKINGENEMY", aCOUNT + 1);
		engine->RegisterEnumValue("Behavior", "ROCKETTURTLEPLUS", aCOUNT + 2);
		engine->RegisterEnumValue("Behavior", "BOLLYTOP", aCOUNT + 3);
		engine->RegisterEnumValue("Behavior", "BOLLYBOTTOM", aCOUNT + 4);
		engine->RegisterEnumValue("Behavior", "PLATFORM", aCOUNT + 5);
		engine->RegisterEnumValue("Behavior", "SPRING", aCOUNT + 6);
		engine->RegisterEnumValue("Behavior", "AMMO15", aCOUNT + 7);
		engine->RegisterEnumValue("Behavior", "MONITOR", aCOUNT + 8);
		engine->RegisterEnumValue("Behavior", "CRATE", aCOUNT + 9);
		engine->RegisterEnumValue("Behavior", "PICKUP", aCOUNT + 10);
		engine->RegisterEnumValue("Behavior", "DIAMONDSAREFOREVER", aCOUNT + 11);
		engine->RegisterEnumValue("Behavior", "FLAG", aCOUNT + 12);
		engine->RegisterEnumValue("Behavior", "MONKEYBULLET", aCOUNT + 13);
		engine->RegisterEnumValue("Behavior", "BILSYBULLET", aCOUNT + 14);
		engine->RegisterEnumValue("Behavior", "BOLLYBULLET", aCOUNT + 15);
		engine->RegisterEnumValue("Behavior", "BOLLYSPIKEBALL", aCOUNT + 16);
		engine->RegisterEnumValue("Behavior", "WITCHBULLET", aCOUNT + 17);
		engine->RegisterEnumValue("Behavior", "TUFBOSSBULLET", aCOUNT + 18);
		engine->RegisterEnumValue("Behavior", "ROBOTSHARD", aCOUNT + 19);
		engine->RegisterEnumValue("Behavior", "BONE", aCOUNT + 20);
		engine->RegisterEnumValue("Behavior", "EXPLOSION2", aCOUNT + 21);
		engine->RegisterEnumValue("Behavior", "BURNING", aCOUNT + 22);
		engine->RegisterEnumValue("Behavior", "AIRBOARDFALL", aCOUNT + 23);
		engine->RegisterEnumValue("Behavior", "BIRDFEATHER", aCOUNT + 24);
		engine->RegisterEnumValue("Behavior", "UFO", aCOUNT + 25);
		engine->RegisterEnumValue("Behavior", "CORPSE", aCOUNT + 26);
		engine->RegisterEnumValue("Behavior", "BIGOBJECT", aCOUNT + 27);

		engine->SetDefaultNamespace("ANIM");
		engine->RegisterEnumValue("Set", "AMMO", mAMMO);
		engine->RegisterEnumValue("Set", "BAT", mBAT);
		engine->RegisterEnumValue("Set", "BEEBOY", mBEEBOY);
		engine->RegisterEnumValue("Set", "BEES", mBEES);
		engine->RegisterEnumValue("Set", "BIGBOX", mBIGBOX);
		engine->RegisterEnumValue("Set", "BIGROCK", mBIGROCK);
		engine->RegisterEnumValue("Set", "BIGTREE", mBIGTREE);
		engine->RegisterEnumValue("Set", "BILSBOSS", mBILSBOSS);
		engine->RegisterEnumValue("Set", "BIRD", mBIRD);
		engine->RegisterEnumValue("Set", "BIRD3D", mBIRD3D);
		engine->RegisterEnumValue("Set", "BOLLPLAT", mBOLLPLAT);
		engine->RegisterEnumValue("Set", "BONUS", mBONUS);
		engine->RegisterEnumValue("Set", "BOSS", mBOSS);
		engine->RegisterEnumValue("Set", "BRIDGE", mBRIDGE);
		engine->RegisterEnumValue("Set", "BUBBA", mBUBBA);
		engine->RegisterEnumValue("Set", "BUMBEE", mBUMBEE);
		engine->RegisterEnumValue("Set", "BUTTERFLY", mBUTTERFLY);
		engine->RegisterEnumValue("Set", "CARROTPOLE", mCARROTPOLE);
		engine->RegisterEnumValue("Set", "CAT", mCAT);
		engine->RegisterEnumValue("Set", "CAT2", mCAT2);
		engine->RegisterEnumValue("Set", "CATERPIL", mCATERPIL);
		engine->RegisterEnumValue("Set", "CHUCK", mCHUCK);
		engine->RegisterEnumValue("Set", "COMMON", mCOMMON);
		engine->RegisterEnumValue("Set", "CONTINUE", mCONTINUE);
		engine->RegisterEnumValue("Set", "DEMON", mDEMON);
		engine->RegisterEnumValue("Set", "DESTSCEN", mDESTSCEN);
		engine->RegisterEnumValue("Set", "DEVAN", mDEVAN);
		engine->RegisterEnumValue("Set", "DEVILDEVAN", mDEVILDEVAN);
		engine->RegisterEnumValue("Set", "DIAMPOLE", mDIAMPOLE);
		engine->RegisterEnumValue("Set", "DOG", mDOG);
		engine->RegisterEnumValue("Set", "DOOR", mDOOR);
		engine->RegisterEnumValue("Set", "DRAGFLY", mDRAGFLY);
		engine->RegisterEnumValue("Set", "DRAGON", mDRAGON);

		engine->RegisterEnumValue("Set", "EVA", mEVA);
		engine->RegisterEnumValue("Set", "FACES", mFACES);

		engine->RegisterEnumValue("Set", "FATCHK", mFATCHK);
		engine->RegisterEnumValue("Set", "FENCER", mFENCER);
		engine->RegisterEnumValue("Set", "FISH", mFISH);
		engine->RegisterEnumValue("Set", "FLAG", mFLAG);
		engine->RegisterEnumValue("Set", "FLARE", mFLARE);
		engine->RegisterEnumValue("Set", "FONT", mFONT);
		engine->RegisterEnumValue("Set", "FROG", mFROG);
		engine->RegisterEnumValue("Set", "FRUITPLAT", mFRUITPLAT);
		engine->RegisterEnumValue("Set", "GEMRING", mGEMRING);
		engine->RegisterEnumValue("Set", "GLOVE", mGLOVE);
		engine->RegisterEnumValue("Set", "GRASSPLAT", mGRASSPLAT);
		engine->RegisterEnumValue("Set", "HATTER", mHATTER);
		engine->RegisterEnumValue("Set", "HELMUT", mHELMUT);

		engine->RegisterEnumValue("Set", "JAZZ", mJAZZ);
		engine->RegisterEnumValue("Set", "JAZZ3D", mJAZZ3D);

		engine->RegisterEnumValue("Set", "JUNGLEPOLE", mJUNGLEPOLE);
		engine->RegisterEnumValue("Set", "LABRAT", mLABRAT);
		engine->RegisterEnumValue("Set", "LIZARD", mLIZARD);
		engine->RegisterEnumValue("Set", "LORI", mLORI);
		engine->RegisterEnumValue("Set", "LORI2", mLORI2);

		engine->RegisterEnumValue("Set", "MENU", mMENU);
		engine->RegisterEnumValue("Set", "MENUFONT", mMENUFONT);

		engine->RegisterEnumValue("Set", "MONKEY", mMONKEY);
		engine->RegisterEnumValue("Set", "MOTH", mMOTH);

		engine->RegisterEnumValue("Set", "PICKUPS", mPICKUPS);
		engine->RegisterEnumValue("Set", "PINBALL", mPINBALL);
		engine->RegisterEnumValue("Set", "PINKPLAT", mPINKPLAT);
		engine->RegisterEnumValue("Set", "PSYCHPOLE", mPSYCHPOLE);
		engine->RegisterEnumValue("Set", "QUEEN", mQUEEN);
		engine->RegisterEnumValue("Set", "RAPIER", mRAPIER);
		engine->RegisterEnumValue("Set", "RAVEN", mRAVEN);
		engine->RegisterEnumValue("Set", "ROBOT", mROBOT);
		engine->RegisterEnumValue("Set", "ROCK", mROCK);
		engine->RegisterEnumValue("Set", "ROCKTURT", mROCKTURT);

		engine->RegisterEnumValue("Set", "SKELETON", mSKELETON);
		engine->RegisterEnumValue("Set", "SMALTREE", mSMALTREE);
		engine->RegisterEnumValue("Set", "SNOW", mSNOW);
		engine->RegisterEnumValue("Set", "SONCSHIP", mSONCSHIP);
		engine->RegisterEnumValue("Set", "SONICSHIP", mSONCSHIP); // Private/deprecated
		engine->RegisterEnumValue("Set", "SONICPLAT", mSONICPLAT);
		engine->RegisterEnumValue("Set", "SPARK", mSPARK);
		engine->RegisterEnumValue("Set", "SPAZ", mSPAZ);
		engine->RegisterEnumValue("Set", "SPAZ2", mSPAZ2);
		engine->RegisterEnumValue("Set", "SPAZ3D", mSPAZ3D);
		engine->RegisterEnumValue("Set", "SPIKEBOLL", mSPIKEBOLL);
		engine->RegisterEnumValue("Set", "SPIKEBOLL3D", mSPIKEBOLL3D);
		engine->RegisterEnumValue("Set", "SPIKEPLAT", mSPIKEPLAT);
		engine->RegisterEnumValue("Set", "SPRING", mSPRING);
		engine->RegisterEnumValue("Set", "STEAM", mSTEAM);

		engine->RegisterEnumValue("Set", "SUCKER", mSUCKER);
		engine->RegisterEnumValue("Set", "TUBETURT", mTUBETURT);
		engine->RegisterEnumValue("Set", "TUFBOSS", mTUFBOSS);
		engine->RegisterEnumValue("Set", "TUFTUR", mTUFTURT);
		engine->RegisterEnumValue("Set", "TURTLE", mTURTLE);
		engine->RegisterEnumValue("Set", "TWEEDLE", mTWEEDLE);
		engine->RegisterEnumValue("Set", "UTERUS", mUTERUS);
		engine->RegisterEnumValue("Set", "VINE", mVINE);
		engine->RegisterEnumValue("Set", "WARP10", mWARP10);
		engine->RegisterEnumValue("Set", "WARP100", mWARP100);
		engine->RegisterEnumValue("Set", "WARP20", mWARP20);
		engine->RegisterEnumValue("Set", "WARP50", mWARP50);

		engine->RegisterEnumValue("Set", "WITCH", mWITCH);
		engine->RegisterEnumValue("Set", "XBILSY", mXBILSY);
		engine->RegisterEnumValue("Set", "XLIZARD", mXLIZARD);
		engine->RegisterEnumValue("Set", "XTURTLE", mXTURTLE);
		engine->RegisterEnumValue("Set", "ZDOG", mZDOG);
		engine->RegisterEnumValue("Set", "ZSPARK", mZSPARK);
		engine->RegisterEnumValue("Set", "PLUS_AMMO", mZZAMMO);
		engine->RegisterEnumValue("Set", "PLUS_BETA", mZZBETA);
		engine->RegisterEnumValue("Set", "PLUS_COMMON", mZZCOMMON);
		engine->RegisterEnumValue("Set", "PLUS_CONTINUE", mZZCONTINUE);

		engine->RegisterEnumValue("Set", "PLUS_FONT", mZZFONT);
		engine->RegisterEnumValue("Set", "PLUS_MENUFONT", mZZMENUFONT);
		engine->RegisterEnumValue("Set", "PLUS_REPLACEMENTS", mZZREPLACEMENTS);
		engine->RegisterEnumValue("Set", "PLUS_RETICLES", mZZRETICLES);
		engine->RegisterEnumValue("Set", "PLUS_SCENERY", mZZSCENERY);
		engine->RegisterEnumValue("Set", "PLUS_WARP", mZZWARP);
		engine->RegisterGlobalFunction("Set get_CUSTOM(uint8)", asFUNCTION(getCustomSetID), asCALL_CDECL);

		engine->SetDefaultNamespace("RABBIT");
		engine->RegisterEnum("Anim");
		engine->RegisterEnumValue("Anim", "AIRBOARD", mJAZZ_AIRBOARD);
		engine->RegisterEnumValue("Anim", "AIRBOARDTURN", mJAZZ_AIRBOARDTURN);
		engine->RegisterEnumValue("Anim", "BUTTSTOMPLAND", mJAZZ_BUTTSTOMPLAND);
		engine->RegisterEnumValue("Anim", "CORPSE", mJAZZ_CORPSE);
		engine->RegisterEnumValue("Anim", "DIE", mJAZZ_DIE);
		engine->RegisterEnumValue("Anim", "DIVE", mJAZZ_DIVE);
		engine->RegisterEnumValue("Anim", "DIVEFIREQUIT", mJAZZ_DIVEFIREQUIT);
		engine->RegisterEnumValue("Anim", "DIVEFIRERIGHT", mJAZZ_DIVEFIRERIGHT);
		engine->RegisterEnumValue("Anim", "DIVEUP", mJAZZ_DIVEUP);
		engine->RegisterEnumValue("Anim", "EARBRACHIATE", mJAZZ_EARBRACHIATE);
		engine->RegisterEnumValue("Anim", "ENDOFLEVEL", mJAZZ_ENDOFLEVEL);
		engine->RegisterEnumValue("Anim", "FALL", mJAZZ_FALL);
		engine->RegisterEnumValue("Anim", "FALLBUTTSTOMP", mJAZZ_FALLBUTTSTOMP);
		engine->RegisterEnumValue("Anim", "FALLLAND", mJAZZ_FALLLAND);
		engine->RegisterEnumValue("Anim", "FIRE", mJAZZ_FIRE);
		engine->RegisterEnumValue("Anim", "FIREUP", mJAZZ_FIREUP);
		engine->RegisterEnumValue("Anim", "FIREUPQUIT", mJAZZ_FIREUPQUIT);
		engine->RegisterEnumValue("Anim", "FROG", mJAZZ_FROG);
		engine->RegisterEnumValue("Anim", "HANGFIREQUIT", mJAZZ_HANGFIREQUIT);
		engine->RegisterEnumValue("Anim", "HANGFIREREST", mJAZZ_HANGFIREREST);
		engine->RegisterEnumValue("Anim", "HANGFIREUP", mJAZZ_HANGFIREUP);
		engine->RegisterEnumValue("Anim", "HANGIDLE1", mJAZZ_HANGIDLE1);
		engine->RegisterEnumValue("Anim", "HANGIDLE2", mJAZZ_HANGIDLE2);
		engine->RegisterEnumValue("Anim", "HANGINGFIREQUIT", mJAZZ_HANGINGFIREQUIT);
		engine->RegisterEnumValue("Anim", "HANGINGFIRERIGHT", mJAZZ_HANGINGFIRERIGHT);
		engine->RegisterEnumValue("Anim", "HELICOPTER", mJAZZ_HELICOPTER);
		engine->RegisterEnumValue("Anim", "HELICOPTERFIREQUIT", mJAZZ_HELICOPTERFIREQUIT);
		engine->RegisterEnumValue("Anim", "HELICOPTERFIRERIGHT", mJAZZ_HELICOPTERFIRERIGHT);
		engine->RegisterEnumValue("Anim", "HPOLE", mJAZZ_HPOLE);
		engine->RegisterEnumValue("Anim", "HURT", mJAZZ_HURT);
		engine->RegisterEnumValue("Anim", "IDLE1", mJAZZ_IDLE1);
		engine->RegisterEnumValue("Anim", "IDLE2", mJAZZ_IDLE2);
		engine->RegisterEnumValue("Anim", "IDLE3", mJAZZ_IDLE3);
		engine->RegisterEnumValue("Anim", "IDLE4", mJAZZ_IDLE4);
		engine->RegisterEnumValue("Anim", "IDLE5", mJAZZ_IDLE5);
		engine->RegisterEnumValue("Anim", "JUMPFIREQUIT", mJAZZ_JUMPFIREQUIT);
		engine->RegisterEnumValue("Anim", "JUMPFIRERIGHT", mJAZZ_JUMPFIRERIGHT);
		engine->RegisterEnumValue("Anim", "JUMPING1", mJAZZ_JUMPING1);
		engine->RegisterEnumValue("Anim", "JUMPING2", mJAZZ_JUMPING2);
		engine->RegisterEnumValue("Anim", "JUMPING3", mJAZZ_JUMPING3);
		engine->RegisterEnumValue("Anim", "LEDGEWIGGLE", mJAZZ_LEDGEWIGGLE);
		engine->RegisterEnumValue("Anim", "LIFT", mJAZZ_LIFT);
		engine->RegisterEnumValue("Anim", "LIFTJUMP", mJAZZ_LIFTJUMP);
		engine->RegisterEnumValue("Anim", "LIFTLAND", mJAZZ_LIFTLAND);
		engine->RegisterEnumValue("Anim", "LOOKUP", mJAZZ_LOOKUP);
		engine->RegisterEnumValue("Anim", "LOOPY", mJAZZ_LOOPY);
		engine->RegisterEnumValue("Anim", "PUSH", mJAZZ_PUSH);
		engine->RegisterEnumValue("Anim", "QUIT", mJAZZ_QUIT);
		engine->RegisterEnumValue("Anim", "REV1", mJAZZ_REV1);
		engine->RegisterEnumValue("Anim", "REV2", mJAZZ_REV2);
		engine->RegisterEnumValue("Anim", "REV3", mJAZZ_REV3);
		engine->RegisterEnumValue("Anim", "RIGHTFALL", mJAZZ_RIGHTFALL);
		engine->RegisterEnumValue("Anim", "RIGHTJUMP", mJAZZ_RIGHTJUMP);
		engine->RegisterEnumValue("Anim", "ROLLING", mJAZZ_ROLLING);
		engine->RegisterEnumValue("Anim", "RUN1", mJAZZ_RUN1);
		engine->RegisterEnumValue("Anim", "RUN2", mJAZZ_RUN2);
		engine->RegisterEnumValue("Anim", "RUN3", mJAZZ_RUN3);
		engine->RegisterEnumValue("Anim", "SKID1", mJAZZ_SKID1);
		engine->RegisterEnumValue("Anim", "SKID2", mJAZZ_SKID2);
		engine->RegisterEnumValue("Anim", "SKID3", mJAZZ_SKID3);
		engine->RegisterEnumValue("Anim", "SPRING", mJAZZ_SPRING);
		engine->RegisterEnumValue("Anim", "STAND", mJAZZ_STAND);
		engine->RegisterEnumValue("Anim", "STATIONARYJUMP", mJAZZ_STATIONARYJUMP);
		engine->RegisterEnumValue("Anim", "STATIONARYJUMPEND", mJAZZ_STATIONARYJUMPEND);
		engine->RegisterEnumValue("Anim", "STATIONARYJUMPSTART", mJAZZ_STATIONARYJUMPSTART);
		engine->RegisterEnumValue("Anim", "STONED", mJAZZ_STONED);
		engine->RegisterEnumValue("Anim", "SWIMDOWN", mJAZZ_SWIMDOWN);
		engine->RegisterEnumValue("Anim", "SWIMRIGHT", mJAZZ_SWIMRIGHT);
		engine->RegisterEnumValue("Anim", "SWIMTURN1", mJAZZ_SWIMTURN1);
		engine->RegisterEnumValue("Anim", "SWIMTURN2", mJAZZ_SWIMTURN2);
		engine->RegisterEnumValue("Anim", "SWIMUP", mJAZZ_SWIMUP);
		engine->RegisterEnumValue("Anim", "SWINGINGVINE", mJAZZ_SWINGINGVINE);
		engine->RegisterEnumValue("Anim", "TELEPORT", mJAZZ_TELEPORT);
		engine->RegisterEnumValue("Anim", "TELEPORTFALL", mJAZZ_TELEPORTFALL);
		engine->RegisterEnumValue("Anim", "TELEPORTFALLING", mJAZZ_TELEPORTFALLING);
		engine->RegisterEnumValue("Anim", "TELEPORTFALLTELEPORT", mJAZZ_TELEPORTFALLTELEPORT);
		engine->RegisterEnumValue("Anim", "TELEPORTSTAND", mJAZZ_TELEPORTSTAND);
		engine->RegisterEnumValue("Anim", "VPOLE", mJAZZ_VPOLE);

		// Create fake MLLE namespace, because "MLLE-Include-xxx.asc" includes are blocked
		engine->SetDefaultNamespace("MLLE");
		engine->RegisterGlobalFunction("bool Setup()", asFUNCTION(mlleSetup), asCALL_CDECL);
		engine->RegisterGlobalProperty("const jjPAL Palette", &jjBackupPalette);
	}

	void LevelScriptLoader::RegisterStandardFunctions(asIScriptEngine* engine, asIScriptModule* module)
	{
		int r;
		r = engine->RegisterGlobalFunction("int Random()", asFUNCTIONPR(asRandom, (), int), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("int Random(int)", asFUNCTIONPR(asRandom, (int), int), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("float Random(float, float)", asFUNCTIONPR(asRandom, (float, float), float), asCALL_CDECL); RETURN_ASSERT(r >= 0);

		r = engine->RegisterGlobalFunction("void Print(const string &in)", asFUNCTION(asScript), asCALL_CDECL); RETURN_ASSERT(r >= 0);

		r = engine->RegisterGlobalFunction("uint8 get_Difficulty()", asFUNCTION(asGetDifficulty), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("bool get_IsReforged()", asFUNCTION(asIsReforged), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("int get_LevelWidth()", asFUNCTION(asGetLevelWidth), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("int get_LevelHeight()", asFUNCTION(asGetLevelHeight), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("float get_ElapsedFrames()", asFUNCTION(asGetElapsedFrames), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("float get_AmbientLight()", asFUNCTION(asGetAmbientLight), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("void set_AmbientLight(float)", asFUNCTION(asSetAmbientLight), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("float get_WaterLevel()", asFUNCTION(asGetWaterLevel), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("void set_WaterLevel(float)", asFUNCTION(asSetWaterLevel), asCALL_CDECL); RETURN_ASSERT(r >= 0);

		r = engine->RegisterGlobalFunction("void PreloadMetadata(const string &in)", asFUNCTION(asPreloadMetadata), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("void RegisterSpawnable(int, const string &in)", asFUNCTION(asRegisterSpawnable), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("void Spawn(int, int, int)", asFUNCTION(asSpawnEvent), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("void Spawn(int, int, int, const array<uint8> &in)", asFUNCTION(asSpawnEventParams), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("void Spawn(const string &in, int, int)", asFUNCTION(asSpawnType), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("void Spawn(const string &in, int, int, const array<uint8> &in)", asFUNCTION(asSpawnTypeParams), asCALL_CDECL); RETURN_ASSERT(r >= 0);

		r = engine->RegisterGlobalFunction("void ChangeLevel(int, const string &in = string())", asFUNCTION(asChangeLevel), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		//r = engine->RegisterGlobalFunction("void MusicPlay(const string &in)", asFUNCTION(asMusicPlay), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("void ShowLevelText(const string &in)", asFUNCTION(asShowLevelText), asCALL_CDECL); RETURN_ASSERT(r >= 0);
		r = engine->RegisterGlobalFunction("void SetWeather(uint8, uint8)", asFUNCTION(asSetWeather), asCALL_CDECL); RETURN_ASSERT(r >= 0);

		// Game-specific classes
		ScriptActorWrapper::RegisterFactory(engine, module);
		ScriptPlayerWrapper::RegisterFactory(engine);
	}

	void LevelScriptLoader::OnException(asIScriptContext* ctx)
	{
		int column; const char* sectionName;
		int lineNumber = ctx->GetExceptionLineNumber(&column, &sectionName);
		DEATH_LOG_CALLBACK(LogLevel::Error, "%s (%i, %i): An exception \"%s\" occurred in \"%s\". Please correct the code and try again.", sectionName, lineNumber, column, ctx->GetExceptionString(), ctx->GetExceptionFunction()->GetDeclaration());
	}

	Actors::ActorBase* LevelScriptLoader::CreateActorInstance(const StringView& typeName)
	{
		auto nullTerminatedTypeName = String::nullTerminatedView(typeName);

		// Create an instance of the ActorBase script class that inherits from the ScriptActorWrapper C++ class
		asITypeInfo* typeInfo = _module->GetTypeInfoByName(nullTerminatedTypeName.data());
		if (typeInfo == nullptr) {
			return nullptr;
		}

		asIScriptObject* obj = reinterpret_cast<asIScriptObject*>(_engine->CreateScriptObject(typeInfo));

		// Get the pointer to the C++ side of the ActorBase class
		ScriptActorWrapper* obj2 = *reinterpret_cast<ScriptActorWrapper**>(obj->GetAddressOfProperty(0));

		// Increase the reference count to the C++ object, as this is what will be used to control the life time of the object from the application side 
		obj2->AddRef();

		// Release the reference to the script side
		obj->Release();

		return obj2;
	}

	const SmallVectorImpl<Actors::Player*>& LevelScriptLoader::GetPlayers() const
	{
		return _levelHandler->_players;
	}

	uint8_t LevelScriptLoader::asGetDifficulty()
	{
		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		return (uint8_t)_this->_levelHandler->_difficulty;
	}

	bool LevelScriptLoader::asIsReforged()
	{
		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		return (uint8_t)_this->_levelHandler->_isReforged;
	}

	int LevelScriptLoader::asGetLevelWidth()
	{
		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		return _this->_levelHandler->_tileMap->LevelBounds().X;
	}

	int LevelScriptLoader::asGetLevelHeight()
	{
		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		return _this->_levelHandler->_tileMap->LevelBounds().Y;
	}

	float LevelScriptLoader::asGetElapsedFrames()
	{
		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		return _this->_levelHandler->_elapsedFrames;
	}

	float LevelScriptLoader::asGetAmbientLight()
	{
		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		return _this->_levelHandler->_ambientLightTarget;
	}

	void LevelScriptLoader::asSetAmbientLight(float value)
	{
		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		_this->_levelHandler->_ambientLightTarget = value;
	}

	float LevelScriptLoader::asGetWaterLevel()
	{
		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		return _this->_levelHandler->_waterLevel;
	}

	void LevelScriptLoader::asSetWaterLevel(float value)
	{
		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		_this->_levelHandler->_waterLevel = value;
	}

	void LevelScriptLoader::asPreloadMetadata(const String& path)
	{
		ContentResolver::Get().PreloadMetadataAsync(path);
	}

	void LevelScriptLoader::asRegisterSpawnable(int eventType, const String& typeName)
	{
		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));

		asITypeInfo* typeInfo = _this->_module->GetTypeInfoByName(typeName.data());
		if (typeInfo == nullptr) {
			return;
		}

		bool added = _this->_eventTypeToTypeInfo.emplace(eventType, typeInfo).second;
		if (added) {
			_this->_levelHandler->EventSpawner()->RegisterSpawnable((EventType)eventType, asRegisterSpawnableCallback);
		}
	}

	std::shared_ptr<Actors::ActorBase> LevelScriptLoader::asRegisterSpawnableCallback(const Actors::ActorActivationDetails& details)
	{
		if (auto levelHandler = dynamic_cast<LevelHandler*>(details.LevelHandler)) {
			auto _this = levelHandler->_scripts.get();
			// Spawn() function with custom event cannot be used in OnLevelLoad(), because _scripts is not assigned yet
			if (_this != nullptr) {
				auto it = _this->_eventTypeToTypeInfo.find((int)details.Type);
				if (it != _this->_eventTypeToTypeInfo.end()) {
					asIScriptObject* obj = reinterpret_cast<asIScriptObject*>(_this->_engine->CreateScriptObject(it->second));
					ScriptActorWrapper* obj2 = *reinterpret_cast<ScriptActorWrapper**>(obj->GetAddressOfProperty(0));
					obj2->AddRef();
					obj->Release();
					obj2->OnActivated(details);
					return std::shared_ptr<Actors::ActorBase>(obj2);
				}
			}
		}
		return nullptr;
	}

	void LevelScriptLoader::asSpawnEvent(int eventType, int x, int y)
	{
		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));

		uint8_t spawnParams[Events::EventSpawner::SpawnParamsSize] { };
		auto actor = _this->_levelHandler->EventSpawner()->SpawnEvent((EventType)eventType, spawnParams, Actors::ActorState::None, Vector3i(x, y, ILevelHandler::MainPlaneZ));
		if (actor != nullptr) {
			_this->_levelHandler->AddActor(actor);
		}
	}

	void LevelScriptLoader::asSpawnEventParams(int eventType, int x, int y, const CScriptArray& eventParams)
	{
		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));

		uint8_t spawnParams[Events::EventSpawner::SpawnParamsSize] { };
		int size = eventParams.GetSize();
		std::memcpy(spawnParams, eventParams.At(0), size);

		auto actor = _this->_levelHandler->EventSpawner()->SpawnEvent((EventType)eventType, spawnParams, Actors::ActorState::None, Vector3i(x, y, ILevelHandler::MainPlaneZ));
		if (actor != nullptr) {
			_this->_levelHandler->AddActor(actor);
		}
	}

	void LevelScriptLoader::asSpawnType(const String& typeName, int x, int y)
	{
		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));

		auto actor = _this->CreateActorInstance(typeName);
		if (actor == nullptr) {
			return;
		}

		uint8_t spawnParams[Events::EventSpawner::SpawnParamsSize] { };
		actor->OnActivated({
			.LevelHandler = _this->_levelHandler,
			.Pos = Vector3i(x, y, ILevelHandler::MainPlaneZ),
			.Params = spawnParams
		});
		_this->_levelHandler->AddActor(std::shared_ptr<Actors::ActorBase>(actor));
	}

	void LevelScriptLoader::asSpawnTypeParams(const String& typeName, int x, int y, const CScriptArray& eventParams)
	{
		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));

		auto actor = _this->CreateActorInstance(typeName);
		if (actor == nullptr) {
			return;
		}

		uint8_t spawnParams[Events::EventSpawner::SpawnParamsSize] { };
		int size = eventParams.GetSize();
		std::memcpy(spawnParams, eventParams.At(0), size);

		actor->OnActivated({
			.LevelHandler = _this->_levelHandler,
			.Pos = Vector3i(x, y, ILevelHandler::MainPlaneZ),
			.Params = spawnParams
		});
		_this->_levelHandler->AddActor(std::shared_ptr<Actors::ActorBase>(actor));
	}

	void LevelScriptLoader::asChangeLevel(int exitType, const String& path)
	{
		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		_this->_levelHandler->BeginLevelChange((ExitType)exitType, path);
	}

	void LevelScriptLoader::asShowLevelText(const String& text)
	{
		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		_this->_levelHandler->ShowLevelText(text);
	}

	void LevelScriptLoader::asSetWeather(uint8_t weatherType, uint8_t intensity)
	{
		auto ctx = asGetActiveContext();
		auto _this = reinterpret_cast<LevelScriptLoader*>(ctx->GetEngine()->GetUserData(EngineToOwner));
		_this->_levelHandler->SetWeather((WeatherType)weatherType, intensity);
	}
}

#endif