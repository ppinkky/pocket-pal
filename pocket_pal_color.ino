/* ============================================================================
   POCKET PAL COLOR  -  Waveshare ESP32-S3-Touch-LCD-2  (2.0", ST7789, landscape)
   Game Boy Color style: sliding carousel menu
   Apps: Dice, Magic 8-Ball, Fish Tank, PET (Tamagotchi)

   LIBRARIES: Adafruit GFX + Adafruit ST7735 and ST7789
   BOARD: ESP32S3 Dev Module, USB CDC On Boot=Enabled, PSRAM=OPI PSRAM
   BUTTONS: GPIO12=left, GPIO14=right, GPIO9=enter, all to GND (pin 13)
   ============================================================================ */

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Preferences.h>

Preferences prefs;

#define TFT_SCK  39
#define TFT_MOSI 38
#define TFT_MISO -1
#define TFT_CS   45
#define TFT_DC   42
#define TFT_RST  -1
#define TFT_BL    1
#define BTN_LEFT  12
#define BTN_RIGHT 14
#define BTN_ENTER  9

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 canvas(320, 240);

enum Mode { MENU, DICE, EBALL, AQUA, PET };
#define NAPPS 4

// ---- palette ----
uint16_t NAVY,WHITE_,CREAM,SHADOW,SKY1,SKY2,SUN;
uint16_t CORAL,CORALD,LCORAL,PURPLE,PURPLED,HLPURP;
uint16_t SKYC,SKYD,WATER1,WATER2,SAND,WEED,FISHC,BELLY;
uint16_t MINT,MINTD,LMINT,PINK,DOGBLK,DOGDARK,TONGUE,COLLAR;
uint16_t RPINK,RPINKD,RPINKL,RCREAM,RROSE,RRUG1,RRUG2,RWOOD;
uint16_t APPCOL[NAPPS];
const char* APPLBL[NAPPS]={"DICE","8-BALL","FISH","PET"};

void initColors(){
  NAVY=tft.color565(43,43,69); WHITE_=tft.color565(255,255,255);
  CREAM=tft.color565(255,246,224); SHADOW=tft.color565(24,20,44);
  SKY1=tft.color565(201,242,255); SKY2=tft.color565(143,215,255);
  SUN=tft.color565(255,217,61);
  CORAL=tft.color565(255,107,107); CORALD=tft.color565(224,72,74);
  LCORAL=tft.color565(255,208,194);
  PURPLE=tft.color565(155,114,207); PURPLED=tft.color565(109,73,166);
  HLPURP=tft.color565(201,166,240);
  SKYC=tft.color565(77,150,255); SKYD=tft.color565(53,111,208);
  WATER1=tft.color565(168,230,255); WATER2=tft.color565(92,192,232);
  SAND=tft.color565(242,209,155); WEED=tft.color565(76,175,98);
  FISHC=tft.color565(255,140,66); BELLY=tft.color565(255,224,179);
  MINT=tft.color565(107,203,119); MINTD=tft.color565(60,150,75);
  LMINT=tft.color565(200,240,210);
  PINK=tft.color565(255,158,196);
  DOGBLK=tft.color565(35,32,38); DOGDARK=tft.color565(55,50,58);
  TONGUE=tft.color565(240,120,140); COLLAR=tft.color565(180,220,60);
  RPINK=tft.color565(255,200,210); RPINKD=tft.color565(180,80,100);
  RPINKL=tft.color565(255,220,228); RCREAM=tft.color565(255,240,235);
  RROSE=tft.color565(220,100,120); RRUG1=tft.color565(255,230,235);
  RRUG2=tft.color565(245,210,218); RWOOD=tft.color565(200,160,120);
  APPCOL[0]=CORAL; APPCOL[1]=PURPLE; APPCOL[2]=SKYC; APPCOL[3]=PINK;
}

// ---- text helpers ----
void ctext(const char*s,int cx,int y,int size,uint16_t col){
  canvas.setTextSize(size);canvas.setTextColor(col);
  int w=(int)strlen(s)*6*size; canvas.setCursor(cx-w/2,y);canvas.print(s);
}
void ctextS(const char*s,int cx,int y,int size,uint16_t col,uint16_t sh){
  canvas.setTextSize(size);int w=(int)strlen(s)*6*size;
  canvas.setTextColor(sh);canvas.setCursor(cx-w/2+2,y+2);canvas.print(s);
  canvas.setTextColor(col);canvas.setCursor(cx-w/2,y);canvas.print(s);
}
void wrapCentered(const char*s,int cx,int y,int maxw,int size,uint16_t col){
  canvas.setTextSize(size);canvas.setTextColor(col);
  int maxc=maxw/(6*size);int lh=8*size+3,cy=y;
  char line[48];int li=0;const char*p=s;char word[24];
  while(*p){while(*p==' ')p++;int wl=0;while(*p&&*p!=' '&&wl<23)word[wl++]=*p++;word[wl]=0;
    if(li&&li+1+wl>maxc){line[li]=0;int w=li*6*size;canvas.setCursor(cx-w/2,cy);canvas.print(line);cy+=lh;li=0;}
    if(li)line[li++]=' ';for(int k=0;k<wl;k++)line[li++]=word[k];}
  if(li){line[li]=0;int w=li*6*size;canvas.setCursor(cx-w/2,cy);canvas.print(line);}
}
uint16_t lerp565(uint16_t a,uint16_t b,float t){
  int ar=(a>>11)&31,ag=(a>>5)&63,ab=a&31,br=(b>>11)&31,bg=(b>>5)&63,bb=b&31;
  return (int)(ar+(br-ar)*t)<<11|(int)(ag+(bg-ag)*t)<<5|(int)(ab+(bb-ab)*t);
}
void vgrad(int y0,int h,uint16_t c1,uint16_t c2){
  for(int i=0;i<h;i++) canvas.drawFastHLine(0,y0+i,320,lerp565(c1,c2,(float)i/(h-1)));
}

// ---- icons ----
const uint8_t PIPS[7][6]={{0,0,0,0,0,0},{4,0,0,0,0,0},{0,8,0,0,0,0},{0,4,8,0,0,0},{0,2,6,8,0,0},{0,2,4,6,8,0},{0,2,3,5,6,8}};
void iconDice(int cx,int cy,int val,int s){
  int x=cx-s/2,y=cy-s/2,r=s*0.2;
  canvas.fillRoundRect(x,y,s,s,r,NAVY);canvas.fillRoundRect(x+3,y+3,s-6,s-6,r-2,WHITE_);
  float cell=(s-14)/3.0;int pr=max(2,(int)(s*0.06));
  if(val<1)val=1;if(val>6)val=6;
  for(int k=0;k<val;k++){int i=PIPS[val][k];
    int gx=x+7+(i%3)*cell+cell/2,gy=y+7+(i/3)*cell+cell/2;
    canvas.fillCircle(gx,gy,pr,CORAL);}
}
void iconBall(int cx,int cy,int s){
  int r=s/2;
  canvas.fillCircle(cx,cy,r,NAVY);canvas.fillCircle(cx,cy,r-3,PURPLE);
  canvas.fillCircle(cx-r*0.35,cy-r*0.35,r*0.28,HLPURP);
  int wy=cy+r*0.12,wr=r*0.52;
  canvas.fillCircle(cx,wy,wr,WHITE_);canvas.drawCircle(cx,wy,wr,NAVY);
  int sz=max(2,s/22);ctext("8",cx,wy-4*sz,sz,NAVY);
}
void iconFish(int cx,int cy,int dir){
  int d=dir;
  canvas.fillTriangle(cx-15*d,cy,cx-27*d,cy-11,cx-27*d,cy+11,NAVY);
  canvas.fillTriangle(cx-14*d,cy,cx-24*d,cy-7,cx-24*d,cy+7,FISHC);
  canvas.fillRoundRect(cx-16,cy-13,32,26,12,NAVY);
  canvas.fillRoundRect(cx-14,cy-11,28,22,10,FISHC);
  canvas.fillRoundRect(cx-11,cy+4,22,7,4,BELLY);
  canvas.fillCircle(cx+9*d,cy-3,5,WHITE_);canvas.fillCircle(cx+10*d,cy-3,2,NAVY);
}
// Pet icon (small, for carousel card)
void iconPet(int cx,int cy){
  // fluffy round head
  canvas.fillCircle(cx,cy,18,NAVY);canvas.fillCircle(cx,cy,16,DOGBLK);
  // floppy ears
  canvas.fillRoundRect(cx-22,cy-6,12,20,6,NAVY);canvas.fillRoundRect(cx-21,cy-5,10,18,5,DOGBLK);
  canvas.fillRoundRect(cx+10,cy-6,12,20,6,NAVY);canvas.fillRoundRect(cx+11,cy-5,10,18,5,DOGBLK);
  // curly fluff on top
  canvas.fillCircle(cx-6,cy-14,5,DOGBLK);canvas.fillCircle(cx+6,cy-14,5,DOGBLK);
  canvas.fillCircle(cx,cy-16,5,DOGBLK);
  // eyes
  canvas.fillCircle(cx-6,cy-2,4,WHITE_);canvas.fillCircle(cx+6,cy-2,4,WHITE_);
  canvas.fillCircle(cx-5,cy-2,2,NAVY);canvas.fillCircle(cx+7,cy-2,2,NAVY);
  // nose
  canvas.fillCircle(cx,cy+4,3,NAVY);
  // tongue
  canvas.fillRoundRect(cx-3,cy+7,6,6,3,TONGUE);
  // collar
  canvas.fillRect(cx-10,cy+12,20,4,COLLAR);
}
void drawIconFor(int idx,int cx,int cy){
  if(idx==0) iconDice(cx,cy,5,58);
  else if(idx==1) iconBall(cx,cy,58);
  else if(idx==2) iconFish(cx,cy,1);
  else iconPet(cx,cy);
}

// ================= state =================
Mode mode=MENU;
int selected=0,slideX=0;
int diceN=2,dieVal[2]={6,1},rollFrames=0;
int shakeFrames=0;bool ballShown=false;const char*ballAns="";
struct FishS{float x,y0,dir,speed,phase,amp;};
FishS fish[8];int nFish=4;
struct Bub{float x,y,vy;bool on;}bub[10];
struct FoodS{float x,y;bool on;}food={0,0,false};

const char*ANSWERS[20]={
  "It is certain","Without a doubt","Yes definitely","You may rely on it",
  "As I see it yes","Most likely","Outlook good","Yes for sure",
  "Signs point to yes","Reply hazy try again","Ask again later",
  "Better not tell you now","Cannot predict now","Concentrate and ask",
  "Do not count on it","My reply is no","My sources say no",
  "Outlook not so good","Very doubtful","Absolutely not"
};

// ---- Pet state ----
float petHappy=80, petHunger=80, petLonely=80;
int petAnim=0;       // 0=idle, 1=pet, 2=treat, 3=call
int petAnimT=0;      // animation timer
float petBounce=0;
int petBlink=0;
uint32_t lastDecay=0;
#define NHEART 6
struct Heart{float x,y,vy;bool on;} hearts[NHEART];
#define NSTAR 6
struct Star{float x,y,vy;bool on;} stars[NSTAR];

void savePet(){
  prefs.begin("pet",false);
  prefs.putFloat("happy",petHappy);
  prefs.putFloat("hunger",petHunger);
  prefs.putFloat("lonely",petLonely);
  prefs.end();
}
void loadPet(){
  prefs.begin("pet",true);
  petHappy=prefs.getFloat("happy",80);
  petHunger=prefs.getFloat("hunger",80);
  petLonely=prefs.getFloat("lonely",80);
  prefs.end();
}

void initFish(int i){fish[i].x=random(30,290);fish[i].y0=random(78,150);
  fish[i].dir=random(0,2)?1:-1;fish[i].speed=random(30,95)/100.0;
  fish[i].phase=random(0,628)/100.0;fish[i].amp=random(4,10);}
void initAquarium(){for(int i=0;i<8;i++)initFish(i);
  for(int i=0;i<10;i++)bub[i].on=false;food.on=false;}
void addFish(){if(nFish<8){initFish(nFish);nFish++;}}
void removeFish(){if(nFish>1)nFish--;}

// ================= CAROUSEL MENU =================
void drawCloud(int x,int y,int s){
  canvas.fillCircle(x,y,s,WHITE_);canvas.fillCircle(x+s,y+3,s*0.8,WHITE_);
  canvas.fillCircle(x-s,y+3,s*0.75,WHITE_);
  canvas.fillRoundRect(x-s*1.6,y,s*3.2,s,s*0.6,WHITE_);
}
void drawCard(int cx,int idx,bool center){
  int w=104,h=center?150:132,y=center?42:50,x=cx-w/2;
  canvas.fillRoundRect(x+5,y+7,w,h,16,SHADOW);
  canvas.fillRoundRect(x,y,w,h,16,APPCOL[idx]);
  canvas.drawRoundRect(x,y,w,h,16,NAVY);canvas.drawRoundRect(x+1,y+1,w-2,h-2,15,NAVY);
  canvas.fillRoundRect(x+9,y+11,w-18,84,12,CREAM);canvas.drawRoundRect(x+9,y+11,w-18,84,12,NAVY);
  drawIconFor(idx,cx,y+11+42);
  int lp=y+h-36;
  canvas.fillRoundRect(x+11,lp,w-22,26,9,WHITE_);canvas.drawRoundRect(x+11,lp,w-22,26,9,NAVY);
  ctext(APPLBL[idx],cx,lp+6,2,NAVY);
}
void drawDots(){
  int y=222,gap=14,x0=160-(NAPPS-1)*gap/2;
  for(int i=0;i<NAPPS;i++){int cx=x0+i*gap;
    if(i==selected)canvas.fillCircle(cx,y,4,NAVY);
    else{canvas.fillCircle(cx,y,4,WHITE_);canvas.drawCircle(cx,y,4,NAVY);}}
}
void renderMenu(){
  vgrad(0,240,SKY1,SKY2);
  canvas.fillCircle(288,32,20,SUN);canvas.drawCircle(288,32,20,NAVY);
  drawCloud(((int)(millis()/60))%400-40,54,13);
  drawCloud(((int)(millis()/90)+200)%420-40,168,11);
  ctextS("POCKET PAL",160,8,3,WHITE_,NAVY);
  slideX=slideX*7/10;if(abs(slideX)<2)slideX=0;
  for(int slot=-2;slot<=2;slot++){
    int idx=((selected+slot)%NAPPS+NAPPS)%NAPPS;
    int x=160+slot*150+slideX;
    if(x<-70||x>390)continue;
    drawCard(x,idx,abs(x-160)<50);
  }
  canvas.fillTriangle(12,120,20,112,20,128,WHITE_);
  canvas.fillTriangle(308,120,300,112,300,128,WHITE_);
  drawDots();
}

// ================= DICE =================
void renderDice(){
  vgrad(0,240,LCORAL,CORAL);
  ctextS("DICE",160,12,3,WHITE_,CORALD);
  if(rollFrames>0){dieVal[0]=1+random(6);dieVal[1]=1+random(6);rollFrames--;}
  int cy=118,s=70;
  if(diceN==1)iconDice(160,cy,dieVal[0],s);
  else{iconDice(160-52,cy,dieVal[0],s);iconDice(160+52,cy,dieVal[1],s);}
  if(rollFrames==0){
    int sum=dieVal[0]+(diceN==2?dieVal[1]:0);
    canvas.fillRoundRect(160-46,186,92,32,10,WHITE_);canvas.drawRoundRect(160-46,186,92,32,10,NAVY);
    char buf[12];snprintf(buf,sizeof(buf),"= %d",sum);ctext(buf,160,193,3,NAVY);
  }
  ctext("ENTER roll   < > count   hold=back",160,226,1,NAVY);
}

// ================= 8-BALL =================
void renderBall(){
  vgrad(0,240,HLPURP,PURPLE);
  ctextS("MAGIC 8-BALL",160,12,2,WHITE_,PURPLED);
  int cx=160,cy=122;
  if(shakeFrames>0){
    iconBall(cx+random(-5,6),cy+random(-5,6),120);
    shakeFrames--;if(shakeFrames==0){ballAns=ANSWERS[random(20)];ballShown=true;}
    ctext("shaking...",160,224,1,WHITE_);return;
  }
  iconBall(cx,cy,120);
  if(ballShown){
    canvas.fillRoundRect(cx-74,cy-26,148,58,12,SKYC);canvas.drawRoundRect(cx-74,cy-26,148,58,12,NAVY);
    wrapCentered(ballAns,cx,cy-16,138,2,WHITE_);
    ctext("ENTER again   hold=back",160,224,1,WHITE_);
  } else ctext("press ENTER to ask   hold=back",160,224,1,WHITE_);
}

// ================= AQUARIUM =================
int nearestFish(){int b=0;float bd=1e9;for(int i=0;i<nFish;i++){
  float d=fabs(fish[i].x-food.x)+fabs(fish[i].y0-food.y);if(d<bd){bd=d;b=i;}}return b;}
void renderAquarium(){
  vgrad(0,240,WATER1,WATER2);
  canvas.fillRect(0,208,320,32,SAND);
  int wx[4]={44,130,214,286};
  for(int k=0;k<4;k++)canvas.fillRoundRect(wx[k],182,8,28,4,WEED);
  ctextS("FISH TANK",160,10,2,WHITE_,SKYD);
  int chaser=food.on?nearestFish():-1;
  for(int i=0;i<nFish;i++){FishS&f=fish[i];
    if(i==chaser){f.dir=(food.x>f.x)?1:-1;f.y0+=(f.y0<food.y?0.8:-0.8);}
    f.x+=f.dir*f.speed*1.4;
    if(f.x<24){f.x=24;f.dir=1;}if(f.x>296){f.x=296;f.dir=-1;}
    f.phase+=0.12;int y=f.y0+sin(f.phase)*f.amp;
    iconFish((int)f.x,y,f.dir>0?1:-1);
    if(food.on&&i==chaser&&fabs(f.x-food.x)<10&&fabs(y-food.y)<10){
      food.on=false;int c=0;
      for(int b=0;b<10&&c<4;b++)if(!bub[b].on){bub[b].on=true;bub[b].x=f.x+c*4;bub[b].y=y;bub[b].vy=0.8+c*0.3;c++;}
    }
  }
  if(food.on){food.y+=0.9;if(food.y>195)food.on=false;
    else{canvas.fillCircle((int)food.x,(int)food.y,3,CORAL);canvas.drawCircle((int)food.x,(int)food.y,3,NAVY);}}
  if(random(100)<14){for(int b=0;b<10;b++)if(!bub[b].on){bub[b].on=true;bub[b].x=random(24,296);bub[b].y=200;bub[b].vy=0.8+random(0,60)/100.0;break;}}
  for(int b=0;b<10;b++){if(!bub[b].on)continue;bub[b].y-=bub[b].vy;
    if(bub[b].y<50){bub[b].on=false;continue;}
    int bx=bub[b].x+sin(bub[b].y*0.1)*3;canvas.drawCircle(bx,(int)bub[b].y,3,WHITE_);}
  ctext("ENTER feed   < > fish   hold=back",160,226,1,NAVY);
}

// ================= PET (Tamagotchi) =================

// pixel heart (5x5 big pixels = 20x20 real px)
void drawHeart(int cx,int cy,uint16_t col,int s){
  int p=s; // pixel size
  // row 0:  .##.##.
  canvas.fillRect(cx-3*p,cy-2*p,p*2,p,col);canvas.fillRect(cx+p,cy-2*p,p*2,p,col);
  // row 1: #######
  canvas.fillRect(cx-3*p,cy-p,p*6,p,col);
  // row 2: .#####.
  canvas.fillRect(cx-2*p,cy,p*4,p,col);
  // row 3: ..###..
  canvas.fillRect(cx-p,cy+p,p*2,p,col);
  // row 4: ...#...
  canvas.fillRect(cx,cy+2*p,p,p,col);
}
// pixel bone
void drawBone(int cx,int cy,int s){
  canvas.fillRect(cx-3*s,cy-s/2,6*s,s,RCREAM);
  canvas.fillRect(cx-4*s,cy-s,s*2,s*2,RCREAM);canvas.fillRect(cx+2*s,cy-s,s*2,s*2,RCREAM);
  canvas.drawRect(cx-3*s,cy-s/2,6*s,s,RPINKD);
}
// pixel phone
void drawPhone(int cx,int cy,int s){
  canvas.fillRect(cx-2*s,cy-3*s,4*s,6*s,NAVY);
  canvas.fillRect(cx-s,cy-2*s,2*s,3*s,SKYC);
  // ring waves
  int t=(millis()/250)%3;
  for(int i=0;i<=t;i++){
    canvas.drawRect(cx-3*s-i*s,cy-4*s-i*s,s,s,RROSE);
    canvas.drawRect(cx+3*s+i*s,cy-4*s-i*s,s,s,RROSE);
  }
}

// Pet sprite: 36x40 hand-tuned pixel art, drawn at 3x (108x120 on screen)
const char* PET_SPR[40] = {
  "................KKKK..K.............",
  "............KKKKBBBBKKBK............",
  "...........KBBBBBBBBBBBBK...........",
  "..........KBBHBBBBBBHBBBHK..........",
  "........KKBBBDBHHBBBDBHHDBKK........",
  ".......KBBBBBBBDBBBBBBDBBBBBK.......",
  "......KBHBBBBHHBBBBBHHBBBBHHHK......",
  "......KBDBBHHDBBHBBBDBBHBBDDBBK.....",
  "......KBBBBDBBBBDBBBBBBDBBBBBBK.....",
  "......KBBBHBBBBHHBBBBHHBBBBBBK......",
  "......KKBBDBBHBDBBBBHDBBBBHHKBK.....",
  ".....KBKBBBBBDBBBBBBDBBBBBDBKBK.....",
  "....KBBBBHHBDDDDHBBBBDDDDBBBBBBKK...",
  "...KBBHBBDBBBWEBDHMBHHWEBBHBBHHBBK..",
  "..KBBBKBBBBBBEEMHMMHMMEEBBDBBKBBBK..",
  "..KBBHBKBHBBBMMHMMHMMHMBBBBBKBBBHK..",
  "..KBBDBKHDBBMMHMnnNNHMMHHHBHKBBBDK..",
  ".KBBBBBKBBBBMHMMNNNNMMHMDBBDKBBBBK..",
  "..KBBBBBKBBBHMMHMNNMMHMMHBBKHHBBBK..",
  "..KHHBBBDKHHMMHKKHMKKMMHDBKDDBHBBK..",
  ".KBDBBBBDDKBBHMMTtTTMMHDBKDDBBDBBK..",
  "..KBBBBBDDKKHBMHTtTTMHBBKBDDHBBBBK..",
  "..KHBBBBDDKKDBBBBTTBDBHHBKDDDBHHBK..",
  "..KDBBBBDDKCCCCCCCCCCCCCCKDDBBDBBK..",
  "..KBBBBBDKccccccccGcccccccKDBHHBBBK.",
  "...KBHHBKHHDDDDDDGGDDDDDDDDKBDBBKK..",
  "....KDBKKDBBBBBBDGgBBDBBBBBKKBBK....",
  ".....KKBBHBBBBBBHHBBBHHBBBBBBKK.....",
  ".....KBBHHBKBBHBDBBHBDBBHKBHBBK.....",
  "....KBBBDBBKBBDBBDDDBBBBDKBDBBBK....",
  "....KBBBBBBKHBBBBDDDBBHBBKBBBBHK....",
  "...KBBBBHBBKBBHHBDDDHBDBBKHHHBDBK...",
  "...KBBBBDBBKBBDBBDDDBBBBBKDDBBBBK...",
  "....KBBBHBBKBBHHBDDDBBBBBKHBHHBK....",
  "....KBBBDBHKBBDBBDDDBBBHBKDBDBBBK...",
  ".....KBBBBDMMMMMMDDMMMMMMKBBBBKK....",
  "......KBBBBMMMMMMDDMMMMMMKHBHK......",
  ".......KKKKMMMMMMKKMMMMMMKKKK.......",
  "...........KKKMKK..KKKMKK...........",
  ".............KKK.....KKK............"
};
const char* PET_TAIL[8] = {
  "..KK..", ".KBHK.", "KBBBK.", "KBHBK.", ".KBBK.", ".KBBK.", "..KBK.", "..KK.."
};
uint16_t petPal(char ch){
  switch(ch){
    case 'K': return tft.color565(20,17,24);   case 'B': return tft.color565(42,38,48);
    case 'D': return tft.color565(28,25,32);   case 'H': return tft.color565(78,72,90);
    case 'M': return tft.color565(60,55,68);   case 'E': return tft.color565(10,10,12);
    case 'W': return WHITE_;                   case 'N': return tft.color565(14,12,18);
    case 'n': return tft.color565(120,114,128);case 'T': return tft.color565(240,120,144);
    case 't': return tft.color565(200,80,104); case 'C': return tft.color565(140,198,63);
    case 'c': return tft.color565(90,154,40);  case 'G': return tft.color565(255,208,64);
    case 'g': return tft.color565(200,150,30);
  }
  return 0;
}
void drawSpr(const char* const* spr,int rows,int cols,int x0,int y0,int S){
  for(int y=0;y<rows;y++) for(int x=0;x<cols;x++){
    char ch=spr[y][x]; if(ch=='.') continue;
    canvas.fillRect(x0+x*S,y0+y*S,S,S,petPal(ch));
  }
}
void drawPetPortrait(int cx,int top,int mood){
  const int S=3; int x0=cx-18*S;
  // tail behind body, wags faster when interacting
  int wagFast=(petAnim==1||petAnim==3);
  int wag=((millis()/(wagFast?90:400))%2)?S:0;
  drawSpr(PET_TAIL,8,6,x0+30*S+wag,top+23*S,S);
  drawSpr(PET_SPR,40,36,x0,top,S);
  // blink: close both eyes
  if(petBlink>0){
    int ex[2]={13,22};
    for(int i=0;i<2;i++){
      canvas.fillRect(x0+ex[i]*S,top+13*S,2*S,2*S,petPal('B'));
      canvas.fillRect(x0+ex[i]*S,top+14*S,2*S,S,petPal('K'));
    }
    petBlink--;
  }
  // not happy: tongue goes away
  if(!(mood>=60||petAnim>0)){
    canvas.fillRect(x0+16*S,top+20*S,4*S,3*S,petPal('M'));
    if(mood<30){   // little frown
      canvas.fillRect(x0+16*S,top+20*S,4*S,S,petPal('K'));
      canvas.fillRect(x0+15*S,top+21*S,S,S,petPal('K'));
      canvas.fillRect(x0+20*S,top+21*S,S,S,petPal('K'));
    }
  }
}

// draw a row of pixel hearts for stat display
void drawHeartRow(int x,int y,float val,uint16_t col){
  int full=(int)(val/20); // 0-5 hearts
  int s=3;
  for(int i=0;i<5;i++){
    uint16_t c=(i<full)?col:SHADOW;
    drawHeart(x+i*14,y,c,s);
  }
}

// checkered rug
void drawRug(int x,int y,int w,int h){
  canvas.fillRect(x-2,y-2,w+4,h+4,RPINKD);
  for(int row=0;row<h;row+=8){
    for(int col=0;col<w;col+=8){
      canvas.fillRect(x+col,y+row,8,8,((row+col)/8)%2?RRUG1:RRUG2);
    }
  }
}

// room furniture (simplified pixel versions)
void drawFurniture(){
  // left: plant on table
  canvas.fillRect(16,120,24,8,RWOOD);  // table top
  canvas.fillRect(20,128,4,20,RWOOD);  // leg
  canvas.fillRect(32,128,4,20,RWOOD);  // leg
  canvas.fillRect(22,106,4,16,WEED);   // stem
  canvas.fillRect(18,100,6,8,WEED);    // leaf
  canvas.fillRect(28,104,6,8,WEED);    // leaf
  canvas.fillRect(20,116,12,6,RCREAM); // pot
  canvas.drawRect(20,116,12,6,RPINKD);
  // right: bed
  canvas.fillRect(270,110,40,40,RCREAM);  // mattress
  canvas.drawRect(270,110,40,40,RPINKD);
  canvas.fillRect(270,110,40,12,RPINKL);  // pillow
  canvas.drawRect(270,110,40,12,RPINKD);
  canvas.fillRect(306,110,4,44,RWOOD);    // headboard
  canvas.fillRect(270,146,40,4,RWOOD);    // frame
  // food bowl (top left)
  canvas.fillRoundRect(30,18,40,20,6,RCREAM);
  canvas.drawRoundRect(30,18,40,20,6,RPINKD);
  canvas.fillRoundRect(38,24,24,8,4,RPINKL);
  canvas.drawRoundRect(38,24,24,8,4,RPINKD);
  drawBone(50,28,2);
  // shower (top right)
  canvas.fillRect(280,16,4,20,RPINKD);
  canvas.fillRect(272,14,20,6,RPINKD);
  canvas.fillRect(272,20,4,4,RPINKD);
  // shower dots
  for(int i=0;i<3;i++) for(int j=0;j<2;j++)
    canvas.fillRect(274+i*6,26+j*6,3,3,RPINKD);
}

void renderPet(){
  // pink room background
  canvas.fillScreen(RPINK);
  // floor
  canvas.fillRect(0,160,320,80,RPINKL);
  canvas.drawFastHLine(0,160,320,RPINKD);

  // room furniture
  drawFurniture();

  // checkered rug (center)
  drawRug(112,156,96,36);

  int cx=160;
  int bodyY=74+(sin(petBounce)>0.6?-3:0);
  int avgMood=(int)((petHappy+petHunger+petLonely)/3);

  if(petBlink==0 && random(200)==0) petBlink=3;

  drawPetPortrait(cx,bodyY,avgMood);
  petBounce+=0.08;

  // floating hearts (pet animation)
  if(petAnim==1 && petAnimT>0){
    for(int i=0;i<NHEART;i++){
      if(!hearts[i].on && random(3)==0){
        hearts[i].on=true;hearts[i].x=cx-30+random(60);hearts[i].y=bodyY+10;hearts[i].vy=0.8+random(30)/10.0;
      }
      if(hearts[i].on){
        hearts[i].y-=hearts[i].vy;hearts[i].x+=sin(hearts[i].y*0.1)*0.5;
        if(hearts[i].y<10){hearts[i].on=false;continue;}
        drawHeart((int)hearts[i].x,(int)hearts[i].y,PINK,3);
      }
    }
    petAnimT--;if(petAnimT==0){petAnim=0;for(int i=0;i<NHEART;i++)hearts[i].on=false;}
  }
  // treat animation (bone + stars)
  if(petAnim==2 && petAnimT>0){
    drawBone(cx+76,bodyY+40,3);
    for(int i=0;i<NSTAR;i++){
      if(!stars[i].on && random(3)==0){
        stars[i].on=true;stars[i].x=cx-30+random(60);stars[i].y=bodyY+10;stars[i].vy=0.5+random(20)/10.0;
      }
      if(stars[i].on){
        stars[i].y-=stars[i].vy;
        if(stars[i].y<8){stars[i].on=false;continue;}
        canvas.fillRect((int)stars[i].x,(int)stars[i].y,4,4,SUN);
        canvas.fillRect((int)stars[i].x+1,(int)stars[i].y-1,2,6,SUN);
        canvas.fillRect((int)stars[i].x-1,(int)stars[i].y+1,6,2,SUN);
      }
    }
    petAnimT--;if(petAnimT==0){petAnim=0;for(int i=0;i<NSTAR;i++)stars[i].on=false;}
  }
  // call animation (phone)
  if(petAnim==3 && petAnimT>0){
    drawPhone(cx+80,bodyY+36,3);
    petAnimT--;if(petAnimT==0)petAnim=0;
  }

  // heart stats (top area, like the reference)
  drawHeartRow(32,46,petHappy,PINK);   // left hearts
  drawHeartRow(222,46,petLonely,PINK);  // right hearts
  // hunger hearts center-bottom
  drawHeartRow(127,46,petHunger,RROSE);

  // action labels (bottom bar, matching reference)
  int by=198;
  // Pet button
  canvas.fillRoundRect(20,by,80,34,6,RCREAM);canvas.drawRoundRect(20,by,80,34,6,RPINKD);
  drawHeart(46,by+10,PINK,2);
  ctext("Pet",60,by+20,1,RPINKD);
  // Treat button
  canvas.fillRoundRect(120,by,80,34,6,RCREAM);canvas.drawRoundRect(120,by,80,34,6,RPINKD);
  drawBone(160,by+10,2);
  ctext("Treat",160,by+20,1,RPINKD);
  // Call button
  canvas.fillRoundRect(220,by,80,34,6,RCREAM);canvas.drawRoundRect(220,by,80,34,6,RPINKD);
  drawPhone(260,by+12,2);
  ctext("Call",260,by+20,1,RPINKD);

  // mood text
  if(avgMood>=70) ctext("*happy pet*",160,8,1,RROSE);
  else if(avgMood>=40) ctext("needs attention...",160,8,1,RPINKD);
  else ctext("your pet is sad!",160,8,1,CORALD);

  // decay
  if(millis()-lastDecay>3000){
    lastDecay=millis();
    petHappy=max(0.0f,petHappy-1.5f);
    petHunger=max(0.0f,petHunger-2.0f);
    petLonely=max(0.0f,petLonely-1.0f);
    savePet();
  }
}

// ================= render dispatch =================
void render(){
  if(mode==MENU) renderMenu();
  else if(mode==DICE) renderDice();
  else if(mode==EBALL) renderBall();
  else if(mode==AQUA) renderAquarium();
  else renderPet();
}

// ================= buttons =================
#define DEBOUNCE 25
#define LONGPRESS 700
bool pL=false,pR=false,pE=false;uint32_t tL=0,tR=0,tE=0;bool longE=false;
bool evLeft,evRight,evEnter,evBack;
void pollButtons(){
  evLeft=evRight=evEnter=evBack=false;uint32_t now=millis();
  bool l=digitalRead(BTN_LEFT)==LOW,r=digitalRead(BTN_RIGHT)==LOW,e=digitalRead(BTN_ENTER)==LOW;
  if(l&&!pL&&now-tL>DEBOUNCE){evLeft=true;tL=now;}
  if(r&&!pR&&now-tR>DEBOUNCE){evRight=true;tR=now;}
  if(e&&!pE){tE=now;longE=false;}
  if(e&&!longE&&now-tE>LONGPRESS){evBack=true;longE=true;}
  if(!e&&pE){if(!longE&&now-tE>DEBOUNCE)evEnter=true;}
  pL=l;pR=r;pE=e;
}
void goMode(Mode m){mode=m;
  if(m==DICE)rollFrames=0;
  else if(m==EBALL){ballShown=false;shakeFrames=0;}
  else if(m==PET){lastDecay=millis();}
}
Mode appForIdx(int i){
  if(i==0)return DICE;if(i==1)return EBALL;if(i==2)return AQUA;return PET;
}
void handleInput(){
  if(mode==MENU){
    if(evLeft){selected=(selected+NAPPS-1)%NAPPS;slideX=-150;}
    if(evRight){selected=(selected+1)%NAPPS;slideX=150;}
    if(evEnter) goMode(appForIdx(selected));
  } else if(mode==DICE){
    if(evBack)mode=MENU;
    if((evLeft||evRight)&&rollFrames==0)diceN=(diceN==1?2:1);
    if(evEnter&&rollFrames==0)rollFrames=14;
  } else if(mode==EBALL){
    if(evBack)mode=MENU;
    if(evEnter&&shakeFrames==0)shakeFrames=12;
  } else if(mode==AQUA){
    if(evBack)mode=MENU;
    if(evLeft)removeFish();
    if(evRight)addFish();
    if(evEnter){food.on=true;food.x=60+random(200);food.y=40;}
  } else { // PET
    if(evBack)mode=MENU;
    if(evLeft){  // pet
      petHappy=min(100.0f,petHappy+15);
      petAnim=1;petAnimT=20;petBlink=4;
      savePet();
    }
    if(evEnter){ // treat
      petHunger=min(100.0f,petHunger+20);
      petHappy=min(100.0f,petHappy+5);
      petAnim=2;petAnimT=25;
      savePet();
    }
    if(evRight){ // call
      petLonely=min(100.0f,petLonely+25);
      petHappy=min(100.0f,petHappy+5);
      petAnim=3;petAnimT=30;petBlink=3;
      savePet();
    }
  }
}

// ================= setup / loop =================
void setup(){
  pinMode(BTN_LEFT,INPUT_PULLUP);pinMode(BTN_RIGHT,INPUT_PULLUP);pinMode(BTN_ENTER,INPUT_PULLUP);
  pinMode(TFT_BL,OUTPUT);digitalWrite(TFT_BL,HIGH);
  SPI.begin(TFT_SCK,TFT_MISO,TFT_MOSI,TFT_CS);
  tft.init(240,320);
  tft.invertDisplay(true);
  tft.setRotation(1);
  tft.setSPISpeed(40000000);
  initColors();
  if(!canvas.getBuffer()){
    tft.fillScreen(tft.color565(200,0,0));
    tft.setCursor(10,20);tft.setTextColor(0xFFFF);tft.setTextSize(2);
    tft.print("No framebuffer.\nEnable PSRAM in\nTools > PSRAM.");
    while(1)delay(1000);
  }
  randomSeed(esp_random());
  initAquarium();
  loadPet();
  lastDecay=millis();
  for(int i=0;i<NHEART;i++)hearts[i].on=false;
  for(int i=0;i<NSTAR;i++)stars[i].on=false;
}
void loop(){
  pollButtons();
  handleInput();
  render();
  tft.drawRGBBitmap(0,0,canvas.getBuffer(),320,240);
}
