#pragma once
#include "d3d9.h"


#pragma pack(push, 8)
// Forward declarations
struct gentity_s;
struct sentient_s;
struct actor_s;
struct Font_s;
struct game_hudelem_s;
struct WeaponDef;
struct snapshot_s;
struct XModel;



struct GfxDrawSurfFields
{
	unsigned __int64 objectId : 16;
	unsigned __int64 reflectionProbeIndex : 8;
	unsigned __int64 customIndex : 5;
	unsigned __int64 materialSortedIndex : 11;
	unsigned __int64 prepass : 2;
	unsigned __int64 primaryLightIndex : 8;
	unsigned __int64 surfType : 4;
	unsigned __int64 primarySortKey : 6;
	unsigned __int64 unused : 4;
};


union __declspec(align(8)) GfxDrawSurf
{
	GfxDrawSurfFields fields;
	unsigned __int64 packed;
};


struct __declspec(align(4)) MaterialInfo
{
	const char* name;
	char gameFlags;
	char sortKey;
	char textureAtlasRowCount;
	char textureAtlasColumnCount;
	GfxDrawSurf drawSurf;
	unsigned int surfaceTypeBits;
	unsigned __int16 hashIndex;
};

struct GfxVertexShaderLoadDef
{
	unsigned int* program;
	unsigned __int16 programSize;
	unsigned __int16 loadForRenderer;
};


struct MaterialVertexShaderProgram
{
	IDirect3DVertexShader9* vs;
	GfxVertexShaderLoadDef loadDef;
};


struct MaterialVertexShader
{
	const char* name;
	MaterialVertexShaderProgram prog;
};

struct MaterialStreamRouting
{
	char source;
	char dest;
};


struct __declspec(align(4)) MaterialVertexStreamRouting
{
	MaterialStreamRouting data[16];
	void* decl[17];
};


struct MaterialVertexDeclaration
{
	char streamCount;
	bool hasOptionalSource;
	bool isLoaded;
	MaterialVertexStreamRouting routing;
};

struct GfxPixelShaderLoadDef
{
	unsigned int* program;
	unsigned __int16 programSize;
	unsigned __int16 loadForRenderer;
};


struct MaterialPixelShaderProgram
{
	IDirect3DPixelShader9* ps;
	GfxPixelShaderLoadDef loadDef;
};


struct MaterialPixelShader
{
	const char* name;
	MaterialPixelShaderProgram prog;
};

struct MaterialArgumentCodeConst
{
	unsigned __int16 index;
	char firstRow;
	char rowCount;
};


union MaterialArgumentDef
{
	const float* literalConst;
	MaterialArgumentCodeConst codeConst;
	unsigned int codeSampler;
	unsigned int nameHash;
};


struct MaterialShaderArgument
{
	unsigned __int16 type;
	unsigned __int16 dest;
	MaterialArgumentDef u;
};


struct __declspec(align(4)) MaterialPass
{
	MaterialVertexDeclaration* vertexDecl;
	MaterialVertexShader* vertexShader;
	MaterialPixelShader* pixelShader;
	char perPrimArgCount;
	char perObjArgCount;
	char stableArgCount;
	char customSamplerFlags;
	MaterialShaderArgument* args;
};


struct MaterialTechnique
{
	const char* name;
	unsigned __int16 flags;
	unsigned __int16 passCount;
	MaterialPass passArray[1];
};


struct MaterialTechniqueSet
{
	const char* name;
	char worldVertFormat;
	bool hasBeenUploaded;
	char unused[1];
	MaterialTechniqueSet* remappedTechniqueSet;
	MaterialTechnique* techniques[59];
};


struct Material
{
	MaterialInfo info;
	char stateBitsEntry[67];
	char textureCount;
	char constantCount;
	char stateBitsCount;
	char stateFlags;
	char cameraRegion;
	MaterialTechniqueSet* techniqueSet;
	void* textureTable;
	void* constantTable;
	void* stateBitsTable;
};


typedef int EntHandle;

/* DemoType */
enum DemoType : __int32
{
	DEMO_TYPE_NONE = 0x0,
	DEMO_TYPE_CLIENT = 0x1,
	DEMO_TYPE_SERVER = 0x2,
};

/* CubemapShot */
enum CubemapShot : __int32
{
	CUBEMAPSHOT_NONE = 0x0,
	CUBEMAPSHOT_RIGHT = 0x1,
	CUBEMAPSHOT_LEFT = 0x2,
	CUBEMAPSHOT_BACK = 0x3,
	CUBEMAPSHOT_FRONT = 0x4,
	CUBEMAPSHOT_UP = 0x5,
	CUBEMAPSHOT_DOWN = 0x6,
	CUBEMAPSHOT_COUNT = 0x7,
};

/* CameraMode */
enum CameraMode : __int32
{
	CAM_NORMAL = 0x0,
	CAM_LINKED = 0x1,
	CAM_VEHICLE = 0x2,
	CAM_VEHICLE_GUNNER = 0x3,
	CAM_TURRET = 0x4,
};

/* pmtype_t */
enum pmtype_t : __int32
{
	PM_NORMAL = 0x0,
	PM_NORMAL_LINKED = 0x1,
	PM_NOCLIP = 0x2,
	PM_UFO = 0x3,
	PM_SPECTATOR = 0x4,
	PM_INTERMISSION = 0x5,
	PM_LASTSTAND = 0x6,
	PM_REVIVEE = 0x7,
	PM_LASTSTAND_TRANSITION = 0x8,
	PM_DEAD = 0x9,
	PM_DEAD_LINKED = 0xA,
};

/* pmflags_t */
enum pmflags_t
{
	PMF_PRONE = 0x1,
	PMF_MANTLE = 0x4,
	PMF_LADDER = 0x8,
	PMF_BACKWARDS_RUN = 0x20,
	PMF_RESPAWNED = 0x400,
	PMF_JUMPING = 0x4000,
	PMF_SPRINTING = 0x8000,
	PMF_VEHICLE_ATTACHED = 0x100000,
};

/* weaponstate_t */
enum weaponstate_t : __int32
{
	WEAPON_READY = 0x0,
	WEAPON_RAISING = 0x1,
	WEAPON_RAISING_ALTSWITCH = 0x2,
	WEAPON_DROPPING = 0x3,
	WEAPON_DROPPING_QUICK = 0x4,
	WEAPON_FIRING = 0x5,
	WEAPON_RECHAMBERING = 0x6,
	WEAPON_RELOADING = 0x7,
	WEAPON_RELOADING_INTERUPT = 0x8,
	WEAPON_RELOAD_START = 0x9,
	WEAPON_RELOAD_START_INTERUPT = 0xA,
	WEAPON_RELOAD_END = 0xB,
	WEAPON_MELEE_CHARGE = 0xC,
	WEAPON_MELEE_INIT = 0xD,
	WEAPON_MELEE_FIRE = 0xE,
	WEAPON_MELEE_END = 0xF,
	WEAPON_OFFHAND_INIT = 0x10,
	WEAPON_OFFHAND_PREPARE = 0x11,
	WEAPON_OFFHAND_HOLD = 0x12,
	WEAPON_OFFHAND_START = 0x13,
	WEAPON_OFFHAND = 0x14,
	WEAPON_OFFHAND_END = 0x15,
	WEAPON_DETONATING = 0x16,
	WEAPON_SPRINT_RAISE = 0x17,
	WEAPON_SPRINT_LOOP = 0x18,
	WEAPON_SPRINT_DROP = 0x19,
	WEAPON_DEPLOYING = 0x1A,
	WEAPON_DEPLOYED = 0x1B,
	WEAPON_BREAKING_DOWN = 0x1C,
	WEAPON_SWIM_IN = 0x1D,
	WEAPON_SWIM_OUT = 0x1E,
	WEAPONSTATES_NUM = 0x1F,
};

/* objectiveState_t */
enum objectiveState_t : __int32
{
	OBJST_EMPTY = 0x0,
	OBJST_ACTIVE = 0x1,
	OBJST_INVISIBLE = 0x2,
	OBJST_DONE = 0x3,
	OBJST_CURRENT = 0x4,
	OBJST_FAILED = 0x5,
	OBJST_NUMSTATES = 0x6,
};

/* visionSetLerpStyle_t */
enum visionSetLerpStyle_t : __int32
{
	VISIONSETLERP_UNDEFINED = 0x0,
	VISIONSETLERP_NONE = 0x1,
	VISIONSETLERP_TO_LINEAR = 0x2,
	VISIONSETLERP_TO_SMOOTH = 0x3,
	VISIONSETLERP_BACKFORTH_LINEAR = 0x4,
	VISIONSETLERP_BACKFORTH_SMOOTH = 0x5,
};

/* markerState_t */
enum markerState_t : __int32
{
	MRKST_EMPTY = 0x0,
	MRKST_ACTIVE_ENTITY = 0x1,
	MRKST_ACTIVE_VECTOR = 0x2,
};

/* entityType_t */
enum entityType_t : __int32
{
	ET_GENERAL = 0x0,
	ET_PLAYER = 0x1,
	ET_PLAYER_CORPSE = 0x2,
	ET_ITEM = 0x3,
	ET_MISSILE = 0x4,
	ET_INVISIBLE = 0x5,
	ET_SCRIPTMOVER = 0x6,
	ET_SOUND_BLEND = 0x7,
	ET_FX = 0x8,
	ET_LOOP_FX = 0x9,
	ET_PRIMARY_LIGHT = 0xA,
	ET_MG42 = 0xB,
	ET_PLANE = 0xC,
	ET_VEHICLE = 0xD,
	ET_VEHICLE_COLLMAP = 0xE,
	ET_VEHICLE_CORPSE = 0xF,
	ET_ACTOR = 0x10,
	ET_ACTOR_SPAWNER = 0x11,
	ET_ACTOR_CORPSE = 0x12,
	ET_EVENTS = 0x13,
};

/* team_t */
enum team_t : __int32
{
	TEAM_FREE = 0x0,
	TEAM_BAD = 0x0,
	TEAM_AXIS = 0x1,
	TEAM_ALLIES = 0x2,
	TEAM_NEUTRAL = 0x3,
	TEAM_DEAD = 0x4,
	TEAM_NUM_TEAMS = 0x5,
};


/* Camera */
struct Camera
{
	float lastViewOrg[3];
	float lastViewAngles[3];
	float tweenStartOrg[3];
	float tweenStartAngles[3];
	float tweenStartFOV;
	int tweenStartTime;
	float tweenDuration;
	float lastViewLockedEntOrg[3];
	CameraMode lastCamMode;
	int lastVehicleSeatPos;
};

/* SprintState */
struct SprintState
{
	int sprintButtonUpRequired;
	int sprintDelay;
	int lastSprintStart;
	int lastSprintEnd;
	int sprintStartMaxLength;
};



enum trType_t : __int32
{
	TR_STATIONARY = 0x0,
	TR_INTERPOLATE = 0x1,
	TR_LINEAR = 0x2,
	TR_LINEAR_STOP = 0x3,
	TR_SINE = 0x4,
	TR_GRAVITY = 0x5,
	TR_ACCELERATE = 0x6,
	TR_DECELERATE = 0x7,
	TR_PHYSICS = 0x8,
	TR_XDOLL = 0x9,
	TR_FIRST_RAGDOLL = 0xA,
	TR_RAGDOLL = 0xA,
	TR_RAGDOLL_GRAVITY = 0xB,
	TR_RAGDOLL_INTERPOLATE = 0xC,
	TR_LAST_RAGDOLL = 0xC,
	TR_COUNT = 0xD,
};


struct trajectory_t
{
	trType_t trType;
	int trTime;
	int trDuration;
	float trBase[3];
	float trDelta[3];
};


struct __declspec(align(4)) LerpEntityState
{
	int eFlags;
	trajectory_t pos;
	trajectory_t apos;
	char u[0x3C];
	int usecount;
};




struct LoopSound
{
	unsigned __int16 soundAlias;
	__int16 fadeTime;
};


enum OffhandSecondaryClass : __int32
{
	PLAYER_OFFHAND_SECONDARY_SMOKE = 0x0,
	PLAYER_OFFHAND_SECONDARY_FLASH = 0x1,
	PLAYER_OFFHAND_SECONDARIES_TOTAL = 0x2,
};

enum ViewLockTypes : __int32
{
	PLAYERVIEWLOCK_NONE = 0x0,
	PLAYERVIEWLOCK_FULL = 0x1,
	PLAYERVIEWLOCK_WEAPONJITTER = 0x2,
	PLAYERVIEWLOCKCOUNT = 0x3,
};

struct MantleState
{
	float yaw;
	int timer;
	int transIndex;
	int flags;
};



/* playerEntity_t */
struct playerEntity_t
{
	float fLastWeaponPosFrac;
	int bPositionToADS;
	float vPositionLastOrg[3];
	float vPositionLastAng[3];
	float fLastIdleFactor;
	float vLastMoveOrg[3];
	float vLastMoveAng[3];
};

struct GfxDepthOfField
{
	float viewModelStart;
	float viewModelEnd;
	float nearStart;
	float nearEnd;
	float farStart;
	float farEnd;
	float nearBlur;
	float farBlur;
};

struct __declspec(align(4)) GfxFilm
{
	float tintDark[3];
	float tintLight[3];
	float brightness;
	float contrast;
	float desaturation;
	bool invert;
	bool enabled;
};

struct __declspec(align(4)) GfxGlow
{
	float godRayPos[2];
	float bloomCutoff;
	float bloomDesaturation;
	float bloomIntensity;
	float radius;
	float rayExpansion;
	float rayIntensity;
	float rayFalloff;
	bool enabled;
};

struct GfxMotionBlur
{
	float viewDirPreviousFrame[3];
	float vieworgPreviousFrame[3];
	float direction[3];
	float radialBlurOrigin[4];
	float motionBlurMagnitude;
	float radialBlurMagnitude;
	int radialBlurStartMSec;
	int radialBlurCurrentTime;
	int radialBlurDuration;
	int radialBlurFadeOutTime;
	float blendWeight;
	int frameBasedEnabled;
	int hasSavedScreen;
	int radialBlurEnabled;
	int isViewDirUpdated;
};

struct GfxDoubleVision
{
	float deltaPerMS;
	float cur;
	float targ;
};

struct __declspec(align(4)) GfxCompositeFx
{
	GfxFilm film;
	float distortionScale[2];
	float blurRadius;
	float distortionMagnitude;
	float frameRate;
	int lastUpdate;
	int frame;
	int startMSec;
	int currentTime;
	int duration;
	bool enabled;
	bool scriptEnabled;
};

struct __declspec(align(4)) GfxReviveFx
{
	GfxFilm centerFilm;
	GfxFilm edgeFilm;
	float motionblurWeight;
	float blurRadius;
	int currentTime;
	bool hasProcessedSavedScreen;
	bool enabled;
};

struct GfxPoison
{
	float curAmountTarget;
	float curAmount;
};

struct GfxLight
{
	unsigned __int8 type;
	unsigned __int8 canUseShadowMap;
	__int16 cullDist;
	float color[3];
	float dir[3];
	float origin[3];
	float radius;
	float cosHalfFovOuter;
	float cosHalfFovInner;
	int exponent;
	unsigned int spotShadowIndex;
	void* def;
};

struct GfxViewport
{
	int x;
	int y;
	int width;
	int height;
};


/* refdef_s */
struct __declspec(align(4)) refdef_s
{
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
	float tanHalfFovX;
	float tanHalfFovY;
	float fov_x;
	float vieworg[3];
	float yaw;
	float viewaxis[3][3];
	float viewOffset[3];
	int time;
	float zNear;
	float blurRadius;
	GfxDepthOfField dof;
	GfxFilm film;
	GfxGlow glow;
	GfxMotionBlur motionBlur;
	GfxDoubleVision doubleVision;
	GfxCompositeFx flameFx;
	GfxReviveFx reviveFx;
	GfxCompositeFx waterSheetingFx;
	GfxPoison poisonFx;
	GfxCompositeFx electrifiedFx;
	GfxCompositeFx transportedFx;
	float sunVisibility;
	GfxLight primaryLights[255];
	GfxViewport scissorViewport;
	bool useScissorViewport;
	int localClientNum;
	int hideMatureContent;
	int splitscreenUsesFullFrameBuffer;
	int splitscreen;
};

/* objective_t */
struct objective_t
{
	objectiveState_t state;
	float origin[3];
	int entNum;
	int teamNum;
	int squadNum;
	int icon;
};


struct colorunpacked
{
	char r;
	char g;
	char b;
	char a;
};


union __declspec(align(4)) hudelem_color_t
{
	colorunpacked color;
	int rgba;
};

/* hudelem_s */
struct hudelem_s
{
	int type;
	float x;
	float y;
	float z;
	int targetEntNum;
	float fontScale;
	float fromFontScale;
	int fontScaleStartTime;
	int fontScaleTime;
	int font;
	int alignOrg;
	int alignScreen;
	hudelem_color_t color;
	hudelem_color_t fromColor;
	int fadeStartTime;
	int fadeTime;
	int label;
	int width;
	int height;
	int materialIndex;
	int offscreenMaterialIdx;
	int fromWidth;
	int fromHeight;
	int scaleStartTime;
	int scaleTime;
	float fromX;
	float fromY;
	int fromAlignOrg;
	int fromAlignScreen;
	int moveStartTime;
	int moveTime;
	int time;
	int duration;
	float value;
	int text;
	float sort;
	hudelem_color_t glowColor;
	int fxBirthTime;
	int fxLetterTime;
	int fxDecayStartTime;
	int fxDecayDuration;
	int soundID;
	int flags;
};

/* playerState_s */
struct __declspec(align(4)) playerState_s
{
	int commandTime;
	pmtype_t pm_type;
	int bobCycle;
	pmflags_t pm_flags;
	int weapFlags;
	int otherFlags;
	int pm_time;
	LoopSound loopSound;
	float origin[3];
	float velocity[3];
	float oldVelocity[2];
	int weaponTime;
	int weaponDelay;
	int grenadeTimeLeft;
	int throwBackGrenadeOwner;
	int throwBackGrenadeTimeLeft;
	int weaponRestrictKickTime;
	bool mountAvailable;
	float mountPos[3];
	float mountDir;
	int foliageSoundTime;
	int gravity;
	float leanf;
	int speed;
	float delta_angles[3];
	int groundEntityNum;
	float vLadderVec[3];
	int jumpTime;
	float jumpOriginZ;
	int legsTimer;
	int legsAnim;
	int torsoTimer;
	int torsoAnim;
	int legsAnimDuration;
	int torsoAnimDuration;
	int damageTimer;
	int damageDuration;
	int flinchYawAnim;
	int corpseIndex;
	int movementDir;
	int eFlags;
	int eventSequence;
	int events[4];
	unsigned int eventParms[4];
	int oldEventSequence;
	int clientNum;
	int offHandIndex;
	OffhandSecondaryClass offhandSecondary;
	unsigned int weapon;
	weaponstate_t weaponstate;
	unsigned int weaponShotCount;
	float fWeaponPosFrac;
	int adsDelayTime;
	int spreadOverride;
	int spreadOverrideState;
	int viewmodelIndex;
	float viewangles[3];
	int viewHeightTarget;
	float viewHeightCurrent;
	int viewHeightLerpTime;
	int viewHeightLerpTarget;
	int viewHeightLerpDown;
	float viewAngleClampBase[2];
	float viewAngleClampRange[2];
	int damageEvent;
	int damageYaw;
	int damagePitch;
	int damageCount;
	int stats[6];
	int ammo[128];
	float heatpercent[128];
	bool overheating[128];
	int ammoclip[128];
	unsigned int weapons[4];
	unsigned int weaponold[4];
	unsigned int weaponrechamber[4];
	float proneDirection;
	float proneDirectionPitch;
	float proneTorsoPitch;
	ViewLockTypes viewlocked;
	int viewlocked_entNum;
	int vehiclePos;
	int vehicleType;
	int vehicleAnimBoneIndex;
	int linkFlags;
	float linkAngles[3];
	float groundTiltAngles[3];
	int cursorHint;
	int cursorHintString;
	int cursorHintEntIndex;
	int iCompassPlayerInfo;
	int radarEnabled;
	int locationSelectionInfo;
	SprintState sprintState;
	float fTorsoPitch;
	float fWaistPitch;
	float holdBreathScale;
	int holdBreathTimer;
	float moveSpeedScaleMultiplier;
	MantleState mantleState;
	int vehicleAnimStage;
	int vehicleEntryPoint;
	unsigned int scriptedAnim;
	int scriptedAnimTime;
	float meleeChargeYaw;
	int meleeChargeDist;
	int meleeChargeTime;
	int weapLockFlags;
	int weapLockedEntnum;
	unsigned int forcedViewAnimWeaponIdx;
	int forcedViewAnimWeaponState;
	unsigned int forcedViewAnimOriginalWeaponIdx;
	int collectibles;
	int actionSlotType[4];
	int actionSlotParams[4];
	int entityEventSequence;
	int weapAnim;
	float aimSpreadScale;
	int shellshockIndex;
	int shellshockTime;
	int shellshockDuration;
	float dofNearStart;
	float dofNearEnd;
	float dofFarStart;
	float dofFarEnd;
	float dofNearBlur;
	float dofFarBlur;
	float dofViewmodelStart;
	float dofViewmodelEnd;
	int waterlevel;
	int hudElemLastAssignedSoundID;
	int artilleryInboundIconLocation;
	objective_t objectives[16];
	char weaponmodels[128];
	int deltatime;
	hudelem_s hudelems[31];
	int perks;
};

/* animCmdState_s */
struct animCmdState_s
{
	int field_0;
	int field_4;
	int field_8;
	int field_C;
	int field_10;
	int field_14;
	int field_18;
	int field_1C;
	int field_20;
	int field_24;
	int field_28;
};


union entityState_index
{
	__int16 brushmodel;
	__int16 xmodel;
	__int16 primaryLight;
	unsigned __int16 bone;
	int pad;
};

union entityState_un1
{
	char destructibleid;
	char pad[4];
};

struct playerAnimState_t
{
	int legsAnim;
	int torsoAnim;
	float fTorsoPitch;
	float fWaistPitch;
};


union __declspec(align(4)) entityState_un2
{
	playerAnimState_t anim;
};

union entityState_un3
{
	int item;
	int hintString;
	int vehicleXModel;
	unsigned int secondBcAlias;
	unsigned int soundTag;
};

/* entityState_s */
struct __declspec(align(4)) entityState_s
{
	int number;
	entityType_t eType;
	LerpEntityState lerp;
	int time2;
	int otherEntityNum;
	int groundEntityNum;
	LoopSound loopSound;
	int surfType;
	entityState_index index;
	int clientnum;
	int iHeadIcon;
	int solid;
	int eventParm;
	int eventSequence;
	int events[4];
	int eventParms[4];
	int weapon;
	int weaponModel;
	int targetname;
	entityState_un1 un1;
	entityState_un2 un2;
	entityState_un3 un3;
	int animtreeIndex;
	int partBits[4];
};

/* clientState_s */
struct __declspec(align(4)) clientState_s
{
	int clientNum;
	team_t team;
	int modelindex;
	int attachModelIndex[6];
	int attachTagIndex[6];
	int lastDamageTime;
	int lastStandStartTime;
	int beingRevived;
	int score;
	int scoreMultiplier;
	char name[32];
	float maxSprintTimeMultiplier;
	int rank;
	int prestige;
	char clanAbbrev[8];
	int attachedEntNum;
	int attachedTagIndex;
	int vehAnimState;
	int perks;
};












/* actorState_s */
struct actorState_s
{
	int actorIndex;
	int entityNum;
	team_t team;
	int modelindex;
	int attachModelIndex[6];
	int attachTagIndex[6];
	char name[32];
	int attachedEntNum;
	int attachedTagIndex;
	int animScriptedAnim;
	int hudwarningType;
	int lookAtEntNum;
	int lastLookAtEntNum;
};


/* snapshot_s */
struct __declspec(align(4)) snapshot_s
{
	int snapFlags;
	int ping;
	int serverTime;
	playerState_s ps;
	int numEntities;
	int numClients;
	int numActors;
	int field_20C4;
	animCmdState_s parseAnimCmds[1117];
	int field_E0C4;
	char what;
	char whatthe[1024];
	char gap_E4C9[3];
	entityState_s parseEntities[1024];
	clientState_s parseClients[4];
	actorState_s parseActors[32];
	int serverCommandSequence;
};

/* score_t */
struct __declspec(align(4)) score_t
{
	int client;
	int score;
	int ping;
	int team;
	int kills;
	int rank;
	int assists;
	int downs;
	int revives;
	int headshots;
	int scoreMultiplier;
	Material* hStatusIcon;
	Material* hRankIcon;
};

/* viewDamage_t */
struct viewDamage_t
{
	int time;
	int duration;
	float yaw;
};

/* objectiveInfo_t */
struct objectiveInfo_t
{
	objectiveState_t state;
	float origin[8][3];
	int centNum[8];
	char string[1024];
	int ringTime;
	int ringToggle;
	int icon;
};

/* targetInfo_t */
struct targetInfo_t
{
	int entNum;
	float offset[3];
	int materialIndex;
	int offscreenMaterialIndex;
	int flags;
};

union cpose_u
{
	char pad[136];
};


/* cpose_t */
struct __declspec(align(4)) cpose_t
{
	unsigned __int16 lightingHandle;
	unsigned __int8 eType;
	unsigned __int8 eTypeUnion;
	unsigned __int8 localClientNum;
	bool isRagdoll;
	int ragdollHandle;
	int physObjId;
	int physUserBody;
	unsigned __int8 destructiblePose;
	int startBurnTime;
	float wetness;
	int cullIn;
	float origin[3];
	float angles[3];
	float mins[3];
	cpose_u u;
};

/* XAnimTree_s */
struct __declspec(align(4)) XAnimTree_s
{
	void* anims;
	unsigned __int16 children;
};

/* centity_s */
struct centity_s
{
	cpose_t pose;
	entityState_s nextState;
	LerpEntityState currentState;
	int previousEventSequence;
	int miscTime;
	int lastMuzzleFlash;
	float lightingOrigin[3];
	XAnimTree_s* tree;
	void* destructible;
	void* nitrousVeh;
	void* scripted;
	void* linkInfo;
	void* vehicleInfo;
	int nextRippleTime;
	int numBulletImpacts;
	unsigned __int16 attachModelNames[2];
	unsigned __int16 attachTagNames[2];
	float originError[3];
	float anglesError[3];
	int firstAnimationTime;
	unsigned __int32 applyLeftHandIK : 1;
	unsigned __int32 nextValid : 1;
	unsigned __int32 bMuzzleFlash : 1;
	unsigned __int32 bTrailMade : 1;
	unsigned __int32 isBurning : 1;
	unsigned __int32 skipBloodImpacts : 1;
	unsigned __int32 scriptThreaded : 1;
	unsigned __int32 clientRumbleLoop : 1;
	unsigned __int32 leftFootstep : 1;
	unsigned __int32 rightFootstep : 1;
	unsigned __int32 didOverheatFx : 1;
	unsigned __int32 originAnglesError : 1;
};

/* visionSetVars_t */
struct visionSetVars_t
{
	bool glowEnable;
	float glowBloomCutoff;
	float glowBloomDesaturation;
	float glowBloomIntensity0;
	float glowBloomIntensity1;
	float glowRadius0;
	float glowRadius1;
	float glowSkyBleedIntensity0;
	float glowSkyBleedIntensity1;
	float glowRayExpansion;
	float glowRayIntensity;
	bool filmEnable;
	float filmBrightness;
	float filmContrast;
	float filmDesaturation;
	bool filmInvert;
	float filmLightTint[3];
	float filmDarkTint[3];
	bool reviveEnable;
	float reviveContrastEdge;
	float reviveBrightnessEdge;
	float reviveDesaturationEdge;
	float reviveDarkTintEdge[3];
	float reviveLightTintEdge[3];
	float reviveBlurRadiusEdge;
	float reviveMotionblurWeight;
	float reviveContrastCenter;
	float reviveBrightnessCenter;
	float reviveDesaturationCenter;
	float reviveDarkTintCenter[3];
	float reviveLightTintCenter[3];
	float masterRingmod;
	float reverbRingmod;
	float hiFilter;
	float lowFilter;
};

/* visionSetLerpData_t */
struct visionSetLerpData_t
{
	int timeStart;
	int timeDuration;
	visionSetLerpStyle_t style;
};

/* hudElemSoundInfo_t */
struct hudElemSoundInfo_t
{
	int lastPlayedTime;
};

/* markerInfo_t */
struct markerInfo_t
{
	markerState_t state;
	char shader[1024];
	float scale;
	int entityNum;
	float vector[3];
};




/* canimscripted_s */
struct __declspec(align(2)) canimscripted_s
{
	float axis[4][3];
	unsigned __int16 anim;
	unsigned __int8 bStarted;
};

/* cLinkInfo_s */
struct cLinkInfo_s
{
	float axis[4][3];
	int linkEnt;
	int linkTag;
};

/* cgVehicle_s */
struct cgVehicle_s
{
	int lastGunnerFire[4];
	int wheelSurfType[6];
	float materialTime;
	void* vehicle_cache;
};









struct cg_s_lastFrame
{
	float aimSpreadScale;
};


/* cg_s - Main client game structure */
struct __declspec(align(4)) cg_s
{
	int clientNum;
	int localClientNum;
	DemoType demoType;
	CubemapShot cubemapShot;
	int cubemapSize;
	int renderScreen;
	int latestSnapshotNum;
	int snapServerTime;
	int loaded;
	snapshot_s* snap;
	snapshot_s* nextSnap;
	snapshot_s activeSnapshots[2];
	centity_s* currTarget;
	XModel* knifeModel;
	float frameInterpolation;
	int frametime;
	int time;
	int time_real;
	int oldTime;
	int physicsTime;
	int mapRestart;
	int renderingThirdPerson;
	void* script_camera;
	playerState_s predictedPlayerState;
	centity_s predictedPlayerEntity;
	playerEntity_t playerEntity;
	int predictedErrorTime;
	float predictedError[3];
	char gapAD064[12];
	float landChange;
	int landTime;
	float heightToCeiling;
	int heightToCeilingTS;
	refdef_s refdef;
	float refdefViewAngles[3];
	Camera cameraData;
	float swayViewAngles[3];
	float swayAngles[3];
	float swayOffset[3];
	float lastStandSwayAngles[3];
	float lastStandSwayAVel[3];
	float lastStandSwayTarget[3];
	int iEntityLastType[2048];
	XModel* pEntityLastXMode[2048];
	bool bEntityDObjDirty[2048];
	int iEntityLastAnimtree[2048];
	float zoomSensitivity;
	char isLoading;
	char objectiveText[1024];
	int vehicleInitView;
	float prevVehicleInvAxis[3][3];
	char vehicleViewLocked;
	float vehicleViewLockedAngles[3];
	char scriptMainMenu[256];
	int scoresRequestTime;
	int numScores;
	score_t scores[4];
	int showScores_real;
	int scoreFadeTime_real;
	int scoresOffBottom;
	int scoresBottom;
	int firstLineVisible;
	int lastLineVisible;
	int drawHud;
	int timeScaleTimeStart;
	float timeScaleStart;
	int timeScaleTimeEnd;
	float timeScaleEnd;
	int crosshairClientNum;
	int crosshairClientLastTime;
	int crosshairClientStartTime;
	int identifyClientNum;
	int deadquoteStartTime;
	int cursorHintIcon;
	int cursorHintTime;
	int cursorHintFade;
	int cursorHintString;
	int lastClipFlashTime;
	int invalidCmdHintType;
	int invalidCmdHintTime;
	int successfulCmdHintType;
	int successfulCmdHintTime;
	int lastHealthPulseTime;
	int lastHealthLerpDelay;
	int lastHealthClient;
	float lastHealth;
	float healthOverlayFromAlpha;
	float healthOverlayToAlpha;
	int healthOverlayPulseTime;
	int healthOverlayPulseDuration;
	int healthOverlayPulsePhase;
	bool healthOverlayHurt;
	int healthOverlayLastHitTime;
	float healthOverlayOldHealth;
	int healthOverlayPulseIndex;
	int proneBlockedEndTime;
	int lastStance;
	int lastStanceChangeTime;
	int lastStanceFlashTime;
	int voiceTime;
	int weaponSelect;
	int weaponSelectTime;
	int weaponLatestPrimaryIdx;
	int prevViewmodelWeapon;
	int equippedOffHand;
	viewDamage_t viewDamage[8];
	int damageTime;
	float damageX;
	float damageY;
	float damageValue;
	float viewFade;
	int waterDropCount;
	int waterDropStartTime;
	int waterDropStopTime;
	int weapIdleTime;
	int nomarks;
	int v_dmg_time;
	float v_dmg_pitch;
	float v_dmg_roll;
	float fBobCycle;
	float xyspeed;
	float kickAVel[3];
	float kickAngles[3];
	float offsetAngles[3];
	char field_B83A0;
	int gunPitch;
	int gunYaw;
	int gunXOfs;
	int gunYOfs;
	int gunZOfs;
	float vGunOffset[3];
	float recoilSpeed[3];
	float vAngOfs[3];
	float flamethrowerYawCap;
	float flamethrowerPitchCap;
	float flamethrowerKickOffset[3];
	float viewModelAxis[4][3];
	char hideViewModel;
	float rumbleScale;
	int compassNorthYaw;
	float compassNorth[2];
	Material* compassMapMaterial;
	float compassMapUpperLeft[2];
	float compassMapWorldSize[2];
	int compassLastTime;
	float compassYaw;
	float compassSpeed;
	int compassFadeTime;
	int healthFadeTime;
	int ammoFadeTime;
	int stanceFadeTime;
	int sprintFadeTime;
	int offhandFadeTime;
	int offhandFlashTime;
	objectiveInfo_t objectiveInfo_t[16];
	int showScores;
	int scoreFadeTime;
	targetInfo_t targets[32];
	char shellshock[0x20];
	int field_BD118;
	int field_BD11C;
	int field_BD120;
	int holdBreathTime;
	int holdBreathInTime;
	int holdBreathDelay;
	float holdBreathFrac;
	int waterBob;
	int bloodLastTime;
	int radarProgress;
	float selectedLocation[2];
	SprintState sprintStates;
	int adsViewErrorDone;
	int inKillCam;
	int field_BD164;
	char bgs[0x91B78];
	char field_14ECE0[45056];
	float vehReticleOffset[2];
	float vehReticleVel[2];
	int vehReticleLockOnStartTime;
	int vehReticleLockOnDuration;
	int vehReticleLockOnEntNum;
	cpose_t viewModelPose;
	visionSetVars_t visionSetPreLoaded[8];
	char visionSetPreLoadedName[512];
	visionSetVars_t visionSetFrom[6];
	visionSetVars_t visionSetTo[6];
	visionSetVars_t visionSetCurrent[6];
	visionSetLerpData_t visionSetLerpData[6];
	char visionNameNaked[64];
	char visionNameNight[64];
	char visionNameVampire0[64];
	char visionNameVampire1[64];
	char visionNameBerserker0[64];
	char visionNameBerserker1[64];
	char visionNameLastStand[64];
	char visionNameDeath[64];
	int extraButtons;
	int lastActionSlotTime;
	int playerTeleported;
	int stepViewStart;
	float stepViewChange;
	cg_s_lastFrame lastFrame;
	int nextRippleTime;
	float zNear;
	float prevLinkedInvQuat[4];
	float linkAnglesFrac;
	char prevLinkAnglesSet;
	hudElemSoundInfo_t hudElemSound[32];
	markerInfo_t markers[8];
	int impactEffectsNext;
	int impactEffects[256];
	char visionsetVampireEnable;
	char visionsetBerserkerEnable;
	char visionsetDeathEnable;
	int generateClientSave;
	int commitClientSave;
	char zapperMenuActive;
};

typedef void* snd_alias_list_t;
struct FxEffectDef
{
	char* name;
	int flags;
	int totalSize;
	int msecLoopingLife;
	int elemDefCountLooping;
	int elemDefCountOneShot;
	int elemDefCountEmission;
	unsigned __int8 efPriority;
	void* elemDefs;
};


enum OffhandClass : __int32
{
	OFFHAND_CLASS_NONE = 0x0,
	OFFHAND_CLASS_FRAG_GRENADE = 0x1,
	OFFHAND_CLASS_SMOKE_GRENADE = 0x2,
	OFFHAND_CLASS_FLASH_GRENADE = 0x3,
	OFFHAND_CLASS_COUNT = 0x4,
};

enum weapType_t : __int32
{
	WEAPTYPE_BULLET = 0x0,
	WEAPTYPE_GRENADE = 0x1,
	WEAPTYPE_PROJECTILE = 0x2,
	WEAPTYPE_BINOCULARS = 0x3,
	WEAPTYPE_GAS = 0x4,
	WEAPTYPE_BOMB = 0x5,
	WEAPTYPE_MINE = 0x6,
	WEAPTYPE_NUM = 0x7,
};


struct WeaponDef
{
	const char* szInternalName;
	const char* szDisplayName;
	const char* szOverlayName;
	XModel* gunXModel[16];
	XModel* handXModel;
	const char* someAnim;
	const char* sidleAnim;
	const char* semptyIdleAnim;
	const char* sfireAnim;
	const char* sholdFireAnim;
	const char* slastShotAnim;
	const char* srechamberAnim;
	const char* smeleeAnim;
	const char* smeleeChargeAnim;
	const char* sreloadAnim;
	const char* sreloadEmptyAnim;
	const char* sreloadStartAnim;
	const char* sreloadEndAnim;
	const char* sraiseAnim;
	const char* sfirstRaiseAnim;
	const char* sdropAnim;
	const char* saltRaiseAnim;
	const char* saltDropAnim;
	const char* squickRaiseAnim;
	const char* squickDropAnim;
	const char* semptyRaiseAnim;
	const char* semptyDropAnim;
	const char* ssprintInAnim;
	const char* ssprintLoopAnim;
	const char* ssprintOutAnim;
	const char* sdeployAnim;
	const char* sbreakdownAnim;
	const char* sdetonateAnim;
	const char* snightVisionWearAnim;
	const char* snightVisionRemoveAnim;
	const char* sadsFireAnim;
	const char* sadsLastShotAnim;
	const char* sadsRechamberAnim;
	const char* sadsUpAnim;
	const char* sadsDownAnim;
	const char* szModeName;
	unsigned __int16 hideTags[8];
	unsigned __int16 notetrackSoundMapKeys[20];
	unsigned __int16 notetrackSoundMapValues[20];
	int playerAnimType;
	weapType_t weapType;
	int weapClass;
	int penetrateType;
	int impactType;
	int inventoryType;
	int fireType;
	int clipType;
	int overheatWeapon;
	float overheatRate;
	float cooldownRate;
	float overheatEndVal;
	int coolWhileFiring;
	OffhandClass offhandClass;
	int stance;
	FxEffectDef* viewFlashEffect;
	FxEffectDef* worldFlashEffect;
	snd_alias_list_t* pickupSound;
	snd_alias_list_t* pickupSoundPlayer;
	snd_alias_list_t* ammoPickupSound;
	snd_alias_list_t* ammoPickupSoundPlayer;
	snd_alias_list_t* projectileSound;
	snd_alias_list_t* pullbackSound;
	snd_alias_list_t* pullbackSoundPlayer;
	snd_alias_list_t* fireSound;
	snd_alias_list_t* fireSoundPlayer;
	snd_alias_list_t* fireLoopSound;
	snd_alias_list_t* fireLoopSoundPlayer;
	snd_alias_list_t* fireStopSound;
	snd_alias_list_t* fireStopSoundPlayer;
	snd_alias_list_t* fireLastSound;
	snd_alias_list_t* fireLastSoundPlayer;
	snd_alias_list_t* emptyFireSound;
	snd_alias_list_t* emptyFireSoundPlayer;
	snd_alias_list_t* crackSound;
	snd_alias_list_t* whizbySound;
	snd_alias_list_t* meleeSwipeSound;
	snd_alias_list_t* meleeSwipeSoundPlayer;
	snd_alias_list_t* meleeHitSound;
	snd_alias_list_t* meleeMissSound;
	snd_alias_list_t* rechamberSound;
	snd_alias_list_t* rechamberSoundPlayer;
	snd_alias_list_t* reloadSound;
	snd_alias_list_t* reloadSoundPlayer;
	snd_alias_list_t* reloadEmptySound;
	snd_alias_list_t* reloadEmptySoundPlayer;
	snd_alias_list_t* reloadStartSound;
	snd_alias_list_t* reloadStartSoundPlayer;
	snd_alias_list_t* reloadEndSound;
	snd_alias_list_t* reloadEndSoundPlayer;
	snd_alias_list_t* rotateLoopSound;
	snd_alias_list_t* rotateLoopSoundPlayer;
	snd_alias_list_t* deploySound;
	snd_alias_list_t* deploySoundPlayer;
	snd_alias_list_t* finishDeploySound;
	snd_alias_list_t* finishDeploySoundPlayer;
	snd_alias_list_t* breakdownSound;
	snd_alias_list_t* breakdownSoundPlayer;
	snd_alias_list_t* finishBreakdownSound;
	snd_alias_list_t* finishBreakdownSoundPlayer;
	snd_alias_list_t* detonateSound;
	snd_alias_list_t* detonateSoundPlayer;
	snd_alias_list_t* nightVisionWearSound;
	snd_alias_list_t* nightVisionWearSoundPlayer;
	snd_alias_list_t* nightVisionRemoveSound;
	snd_alias_list_t* nightVisionRemoveSoundPlayer;
	snd_alias_list_t* altSwitchSound;
	snd_alias_list_t* altSwitchSoundPlayer;
	snd_alias_list_t* raiseSound;
	snd_alias_list_t* raiseSoundPlayer;
	snd_alias_list_t* firstRaiseSound;
	snd_alias_list_t* firstRaiseSoundPlayer;
	snd_alias_list_t* putawaySound;
	snd_alias_list_t* putawaySoundPlayer;
	snd_alias_list_t* overheatSound;
	snd_alias_list_t* overheatSoundPlayer;
	snd_alias_list_t** bounceSound;
	WeaponDef* standMountedWeapdef;
	WeaponDef* crouchMountedWeapdef;
	WeaponDef* proneMountedWeapdef;
	int StandMountedIndex;
	int CrouchMountedIndex;
	int ProneMountedIndex;
	FxEffectDef* viewShellEjectEffect;
	FxEffectDef* worldShellEjectEffect;
	FxEffectDef* viewLastShotEjectEffect;
	FxEffectDef* worldLastShotEjectEffect;
	Material* reticleCenter;
	Material* reticleSide;
	int iReticleCenterSize;
	int iReticleSideSize;
	int iReticleMinOfs;
	int activeReticleType;
	float vStandMove[3];
	float vStandRot[3];
	float vDuckedOfs[3];
	float vDuckedMove[3];
	float duckedSprintOfs[3];
	float duckedSprintRot[3];
	float duckedSprintBob[2];
	float duckedSprintScale;
	float sprintOfs[3];
	float sprintRot[3];
	float sprintBob[2];
	float sprintScale;
	float vDuckedRot[3];
	float vProneOfs[3];
	float vProneMove[3];
	float vProneRot[3];
	float fPosMoveRate;
	float fPosProneMoveRate;
	float fStandMoveMinSpeed;
	float fDuckedMoveMinSpeed;
	float fProneMoveMinSpeed;
	float fPosRotRate;
	float fPosProneRotRate;
	float fStandRotMinSpeed;
	float fDuckedRotMinSpeed;
	float fProneRotMinSpeed;
	XModel* worldModel[16];
	XModel* worldClipModel;
	XModel* rocketModel;
	XModel* knifeModel;
	XModel* worldKnifeModel;
	XModel* mountedModel;
	Material* hudIcon;
	int hudIconRatio;
	Material* ammoCounterIcon;
	int ammoCounterIconRatio;
	int ammoCounterClip;
	int iStartAmmo;
	const char* szAmmoName;
	int iAmmoIndex;
	const char* szClipName;
	int iClipIndex;
	int iHeatIndex;
	int iMaxAmmo;
	int iClipSize;
	int shotCount;
	const char* szSharedAmmoCapName;
	int iSharedAmmoCapIndex;
	int iSharedAmmoCap;
	int unlimitedAmmo;
	int damage;
	int damageDuration;
	int damageInterval;
	int playerDamage;
	int iMeleeDamage;
	int iDamageType;
	int iFireDelay;
	int iMeleeDelay;
	int meleeChargeDelay;
	int iDetonateDelay;
	int iFireTime;
	int iRechamberTime;
	int iRechamberBoltTime;
	int iHoldFireTime;
	int iDetonateTime;
	int iMeleeTime;
	int meleeChargeTime;
	int iReloadTime;
	int reloadShowRocketTime;
	int iReloadEmptyTime;
	int iReloadAddTime;
	int reloadEmptyAddTime;
	int iReloadStartTime;
	int iReloadStartAddTime;
	int iReloadEndTime;
	int iDropTime;
	int iRaiseTime;
	int iAltDropTime;
	int iAltRaiseTime;
	int quickDropTime;
	int quickRaiseTime;
	int iFirstRaiseTime;
	int iEmptyRaiseTime;
	int iEmptyDropTime;
	int sprintInTime;
	int sprintLoopTime;
	int sprintOutTime;
	int deployTime;
	int breakdownTime;
	int nightVisionWearTime;
	int nightVisionWearTimeFadeOutEnd;
	int nightVisionWearTimePowerUp;
	int nightVisionRemoveTime;
	int nightVisionRemoveTimePowerDown;
	int nightVisionRemoveTimeFadeInStart;
	int fuseTime;
	int aiFuseTime;
	int requireLockonToFire;
	int noAdsWhenMagEmpty;
	int avoidDropCleanup;
	float autoAimRange;
	float aimAssistRange;
	float aimAssistRangeAds;
	int mountableWeapon;
	float aimPadding;
	float enemyCrosshairRange;
	int crosshairColorChange;
	float moveSpeedScale;
	float adsMoveSpeedScale;
	float sprintDurationScale;
	float fAdsZoomFov;
	float fAdsZoomInFrac;
	float fAdsZoomOutFrac;
	Material* overlayMaterial;
	Material* overlayMaterialLowRes;
	int overlayReticle;
	int overlayInterface;
	float overlayWidth;
	float overlayHeight;
	float fAdsBobFactor;
	float fAdsViewBobMult;
	float fHipSpreadStandMin;
	float fHipSpreadDuckedMin;
	float fHipSpreadProneMin;
	float hipSpreadStandMax;
	float hipSpreadDuckedMax;
	float hipSpreadProneMax;
	float fHipSpreadDecayRate;
	float fHipSpreadFireAdd;
	float fHipSpreadTurnAdd;
	float fHipSpreadMoveAdd;
	float fHipSpreadDuckedDecay;
	float fHipSpreadProneDecay;
	float fHipReticleSidePos;
	int iAdsTransInTime;
	int iAdsTransOutTime;
	float fAdsIdleAmount;
	float fHipIdleAmount;
	float adsIdleSpeed;
	float hipIdleSpeed;
	float fIdleCrouchFactor;
	float fIdleProneFactor;
	float fGunMaxPitch;
	float fGunMaxYaw;
	float swayMaxAngle;
	float swayLerpSpeed;
	float swayPitchScale;
	float swayYawScale;
	float swayHorizScale;
	float swayVertScale;
	float swayShellShockScale;
	float adsSwayMaxAngle;
	float adsSwayLerpSpeed;
	float adsSwayPitchScale;
	float adsSwayYawScale;
	float adsSwayHorizScale;
	float adsSwayVertScale;
	int bRifleBullet;
	int armorPiercing;
	int bBoltAction;
	int aimDownSight;
	int bRechamberWhileAds;
	float adsViewErrorMin;
	float adsViewErrorMax;
	int bCookOffHold;
	int bClipOnly;
	int canUseInVehicle;
	int noDropsOrRaises;
	int adsFireOnly;
	int cancelAutoHolsterWhenEmpty;
	int suppressAmmoReserveDisplay;
	int enhanced;
	int laserSightDuringNightvision;
	int bayonet;
	Material* killIcon;
	int killIconRatio;
	int flipKillIcon;
	Material* dpadIcon;
	int dpadIconRatio;
	int bNoPartialReload;
	int bSegmentedReload;
	int noADSAutoReload;
	int iReloadAmmoAdd;
	int iReloadStartAdd;
	const char* szAltWeaponName;
	unsigned int altWeaponIndex;
	int iDropAmmoMin;
	int iDropAmmoMax;
	int blocksProne;
	int silenced;
	int iExplosionRadius;
	int iExplosionRadiusMin;
	int iExplosionInnerDamage;
	int iExplosionOuterDamage;
	float damageConeAngle;
	int iProjectileSpeed;
	int iProjectileSpeedUp;
	int iProjectileSpeedForward;
	int iProjectileActivateDist;
	float projLifetime;
	float timeToAccelerate;
	float projectileCurvature;
	XModel* projectileModel;
	int projExplosion;
	FxEffectDef* projExplosionEffect;
	int projExplosionEffectForceNormalUp;
	FxEffectDef* projDudEffect;
	snd_alias_list_t* projExplosionSound;
	snd_alias_list_t* projDudSound;
	snd_alias_list_t* mortarShellSound;
	snd_alias_list_t* tankShellSound;
	int bProjImpactExplode;
	int stickiness;
	int hasDetonator;
	int timedDetonation;
	int rotate;
	int holdButtonToThrow;
	int freezeMovementWhenFiring;
	float lowAmmoWarningThreshold;
	float parallelBounce[31];
	float perpendicularBounce[31];
	FxEffectDef* projTrailEffect;
	float vProjectileColor[3];
	int guidedMissileType;
	float maxSteeringAccel;
	int projIgnitionDelay;
	FxEffectDef* projIgnitionEffect;
	snd_alias_list_t* projIgnitionSound;
	float fAdsAimPitch;
	float fAdsCrosshairInFrac;
	float fAdsCrosshairOutFrac;
	int adsGunKickReducedKickBullets;
	float adsGunKickReducedKickPercent;
	float fAdsGunKickPitchMin;
	float fAdsGunKickPitchMax;
	float fAdsGunKickYawMin;
	float fAdsGunKickYawMax;
	float fAdsGunKickAccel;
	float fAdsGunKickSpeedMax;
	float fAdsGunKickSpeedDecay;
	float fAdsGunKickStaticDecay;
	float fAdsViewKickPitchMin;
	float fAdsViewKickPitchMax;
	float fAdsViewKickYawMin;
	float fAdsViewKickYawMax;
	float fAdsViewKickCenterSpeed;
	float fAdsViewScatterMin;
	float fAdsViewScatterMax;
	float fAdsSpread;
	int hipGunKickReducedKickBullets;
	float hipGunKickReducedKickPercent;
	float fHipGunKickPitchMin;
	float fHipGunKickPitchMax;
	float fHipGunKickYawMin;
	float fHipGunKickYawMax;
	float fHipGunKickAccel;
	float fHipGunKickSpeedMax;
	float fHipGunKickSpeedDecay;
	float fHipGunKickStaticDecay;
	float fHipViewKickPitchMin;
	float fHipViewKickPitchMax;
	float fHipViewKickYawMin;
	float fHipViewKickYawMax;
	float fHipViewKickCenterSpeed;
	float fHipViewScatterMin;
	float fHipViewScatterMax;
	float fightDist;
	float maxDist;
	const char* accuracyGraphName[2];
	float (*accuracyGraphKnots[2])[2];
	float (*originalAccuracyGraphKnots[2])[2];
	int accuracyGraphKnotCount[2];
	int originalAccuracyGraphKnotCount[2];
	int iPositionReloadTransTime;
	float leftArc;
	float rightArc;
	float topArc;
	float bottomArc;
	float accuracy;
	float aiSpread;
	float playerSpread;
	float minTurnSpeed[2];
	float maxTurnSpeed[2];
	float pitchConvergenceTime;
	float yawConvergenceTime;
	float suppressTime;
	float maxRange;
	float fAnimHorRotateInc;
	float fPlayerPositionDist;
	const char* szUseHintString;
	const char* dropHintString;
	int iUseHintStringIndex;
	int dropHintStringIndex;
	float horizViewJitter;
	float vertViewJitter;
	const char* szScript;
	float fOOPosAnimLength[2];
	int minDamage;
	int minPlayerDamage;
	float fMaxDamageRange;
	float fMinDamageRange;
	float destabilizationRateTime;
	float destabilizationCurvatureMax;
	int destabilizeDistance;
	float locNone;
	float locHelmet;
	float locHead;
	float locNeck;
	float locTorsoUpper;
	float locTorsoLower;
	float locRightArmUpper;
	float locLeftArmUpper;
	float locRightArmLower;
	float locLeftArmLower;
	float locRightHand;
	float locLeftHand;
	float locRightLegUpper;
	float locLeftLegUpper;
	float locRightLegLower;
	float locLeftLegLower;
	float locRightFoot;
	float locLeftFoot;
	float locGun;
	const char* fireRumble;
	const char* meleeImpactRumble;
	float adsDofStart;
	float adsDofEnd;
	float hipDofStart;
	float hipDofEnd;
	const char* flameTableFirstPerson;
	const char* flameTableThirdPerson;
	void* flameTableFirstPersonPtr;
	void* flameTableThirdPersonPtr;
	FxEffectDef* tagFx_preparationEffect;
	FxEffectDef* tagFlash_preparationEffect;
};

struct entityShared_s
{
	unsigned __int8 linked;
	unsigned __int8 bmodel;
	unsigned __int16 svFlags;
	unsigned __int8 eventType;
	unsigned __int8 inuse;
	int clientMask[2];
	int broadcastTime;
	float mins[3];
	float maxs[3];
	int contents;
	float absmin[3];
	float absmax[3];
	float currentOrigin[3];
	float currentAngles[3];
	int ownerNum;
	int eventTime;
};

struct usercmd_s {
	char padding[0x38];
};

struct __declspec(align(4)) clientSession_s
{
	int sessionState;
	int forceSpectatorClient;
	int status_icon;
	int archiveTime;
	int score;
	int kills;
	int assists;
	int downs;
	int revives;
	int headshots;
	int rankxp;
	int something;
	__int16 scriptId;
	char gap_32[2];
	int connected;
	usercmd_s cmd;
	usercmd_s oldcmd;
	int localClient;
	int predictItemPickup;
	char newnetname[32];
	int maxHealth;
	int enterTime;
	int teamState;
	int voteCount;
	int teamVoteCount;
	float moveSpeedScaleMultiplier;
	int viewmodelIndex;
	int noSpectate;
	int teamInfo;
	clientState_s cs;
	int psOffsetTime;
};


struct __declspec(align(4)) gclient_s
{
	playerState_s ps;
	clientSession_s sess;
	int spectatorClient;
	int noclip;
	int ufo;
	int bFrozen;
	int buttons;
	int oldbuttons;
	int latched_buttons;
	int buttonsSinceLastFrame;
	float fGunPitch;
	float fGunYaw;
	float fGunXOfs;
	float fGunYOfs;
	float fGunZOfs;
	int damage_blood;
	float damage_from[3];
	int damage_fromWorld;
	int respawnTime;
	int lastBadArcCreateTime;
	int outWaterTime;
	float currentAimSpreadScale;
	gentity_s* pHitHitEnt;
	EntHandle pLookatEnt;
	float prevLinkedInvQuat[4];
	bool prevLinkAnglesSet;
	bool link_doCollision;
	bool linkAnglesLocked;
	float linkAnglesFrac;
	float linkAnglesMinClamp[2];
	float linkAnglesMaxClamp[2];
	int inControlTime;
	int lastTouchTime;
	EntHandle useHoldEntity;
	int useHoldTime;
	int useButtonDone;
	int bDisableAutoPickup;
	int invulnerableExpireTime;
	bool invulnerableActivated;
	bool invulnerableEnabled;
	bool playerMoved;
	float playerLOSCheckPos[2];
	float playerLOSCheckDir[2];
	int playerLOSPosTime;
	int playerADSTargetTime;
	unsigned int lastWeapon;
	bool previouslyFiring;
	bool previouslyUsingNightVision;
	int groundTiltEntNum;
	int revive;
	int reviveTime;
	int lastStand;
	int lastStandTime;
	int switchSeatTime;
	int lastCmdTime;
	int inactivityTime;
	int inactivityWarning;
	int lastVoiceTime;
	int lastServerTime;
	int lastSpawnTime;
	int damageTime;
	float vGunSpeed[3];
	int dropWeaponTime;
	bool previouslyChangingWeapon;
};

struct flame_timed_damage_t
{
	gentity_s* attacker;
	int damage;
	float damageDuration;
	float damageInterval;
	int start_timestamp;
	int end_timestamp;
	int lastupdate_timestamp;
};

union __declspec(align(4)) gentity_u
{
	char padding[0x60];
};

struct snd_wait_t
{
	unsigned __int16 notifyString;
	unsigned __int16 index;
	unsigned __int8 stoppable;
	int basetime;
	int duration;
};

struct tagInfo_s
{
	gentity_s* parent;
	gentity_s* next;
	unsigned __int16 name;
	int index;
	float axis[4][3];
	float parentInvAxis[4][3];
};

struct animscripted_s
{
	float axis[4][3];
	float originError[3];
	float anglesError[3];
	unsigned __int16 anim;
	unsigned __int16 root;
	unsigned __int8 bStarted;
	unsigned __int8 mode;
	int startTime;
	float fHeightOfs;
	float fEndPitch;
	float fEndRoll;
	float fOrientLerp;
};

enum classNum_e : __int32
{
	CLASS_NUM_ENTITY = 0x0,
	CLASS_NUM_HUDELEM = 0x1,
	CLASS_NUM_PATHNODE = 0x2,
	CLASS_NUM_VEHICLENODE = 0x3,
	CLASS_NUM_COUNT = 0x4,
};


struct __declspec(align(8)) gentity_s
{
	entityState_s s;
	entityShared_s r;
	gclient_s* client;
	actor_s* actor;
	sentient_s* sentient;
	void* scr_vehicle;
	void* pTurretInfo;
	void* destructible;
	unsigned __int16 model;
	unsigned __int8 physicsObject;
	unsigned __int8 takedamage;
	unsigned __int8 active;
	unsigned __int8 nopickup;
	unsigned __int8 handler;
	unsigned __int16 classname;
	unsigned __int16 script_linkName;
	unsigned __int16 script_noteworthy;
	unsigned __int16 target;
	int targetname;
	int spawnflags2;
	int spawnflags;
	int flags;
	int clipmask;
	int processedFrame;
	EntHandle parent;
	int nextthink;
	int health;
	int maxhealth;
	int nexteq;
	int damage;
	flame_timed_damage_t flame_timed_damage[4];
	int last_timed_radius_damage;
	int count;
	gentity_s* chain;
	gentity_s* activator;
	gentity_u u;
	EntHandle missileTargetEnt;
	__int16 lookAtText0;
	__int16 lookAtText1;
	snd_wait_t snd_wait;
	tagInfo_s* tagInfo;
	gentity_s* tagChildren;
	animscripted_s* scripted;
	__int16 attachTagNames[31];
	__int16 attachModelNames[31];
	int disconnectedLinks;
	int iDisconnectTime;
	float angleLerpRate;
	int physObjId;
	XAnimTree_s* pAnimTree;
	gentity_s* nextFree;
	int scriptUse;
	int birthTime;
};


#pragma pack(pop)