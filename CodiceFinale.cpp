/*///////////////////////////////////////////////////
record pallina da tennis 284km/h -> 78,8m/s
30hz caso peggiore vedo la pallina ogni 2,62m
60hz ogni 1,3m
120hz ogni 0,65m
lunghezza campo 23,77m
//////////////////////////////////////////////////*/
#include <iostream>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>

using namespace std;
using namespace cv;

const Scalar SCALAR_BLUE = Scalar(255.0, 0.0, 0.0);
const Scalar SCALAR_RED = Scalar(0.0, 0.0, 255.0);
const Scalar SCALAR_BLACK = Scalar(0.0, 0.0, 0.0);

Rect rectL; //rettangoli regione di interesse
Rect rectR;
int line_vertical1, line_vertical2, line_vertical3, line_vertical4, line_vertical_mask1, line_vertical_mask2; //da 1 a 4 rispettivamente da sinistra a destra e da sopra a sotto
int line_horizontal1, line_horizontal2, line_horizontal_mask;

int maxFrameToLost;
int minDeltaX, minDeltaY, maxDeltaX, maxDeltaY;
int contourAreaMin, contourAreaMax;
float circularityMin;

bool notballSX = false;
bool notballDX = false;
bool ballInfoDX = false;
bool ballInfoSX = false;
bool play = true;
bool data_readySX = false;
bool data_readyDX = false;
bool arrivedSX = false;
bool arrivedDX = false;

double arr_speedSX, part_speedSX, arr_speedDX, part_speedDX;
double applied_forceSX, applied_forceDX;

vector<vector<Point>> contours;

vector<Point> possibleBallInfoSX;
vector<Point> possibleBallInfoDX;

vector<int> framePassedSX;
vector<int> framePassedDX;

int videoSelect = 0;
int numFrame = 0;
int curFrameDX = 0;
int curFrameSX = 0;
int frameLoss = 0;
int ballFrameDX = 0;
int ballFrameSX =0;
int ballValueDX = 0;
int ballValueSX = 0;
int arrCountDX = 0;
int arrCountSX = 0;
int passingTime;

void blobCenter(vector<vector<Point>> &points);
void ballLocationSX(Point &ball);
void ballLocationDX(Point &ball);
void ballTrackingSX(bool possibleBall);
void ballTrackingDX(bool possibleBall);

Mat frame_resized;
Mat frame1, fgMask, modif;

int main(int argc, const char * argv[]) {
  VideoCapture cap;
  int width=0;
  int height=0;
  int fps=0;

  cout<<"Select the video number [1-3]: ";
  cin>>videoSelect;
  cout<<endl;
  while(videoSelect != 1 && videoSelect != 2 && videoSelect != 3){
    cout<<"Error! Must insert a number from 1 to 3"<<endl;
    cout<<"Select the video number [1-3]: ";
    cin>>videoSelect;
    cout<<endl;
  }
  if(videoSelect == 1){ //video 1080p 104_0001_narrowFOV
    cap.open("Video1.MP4");
    if(!cap.isOpened()){
      cout << "Error opening video stream or file" << endl;
      return -1;
    }

    width=cap.get(CAP_PROP_FRAME_WIDTH);
    height=cap.get(CAP_PROP_FRAME_HEIGHT);
    fps=cap.get(CAP_PROP_FPS);

    cout<<"Frame width: "<<width<<endl;
    cout<<"Frame height: "<<height<<endl;
    cout<<"FPS: "<<fps<<endl;

    rectL = Rect(240, 150, 240, 450);
    rectR = Rect(1440, 150, 240, 450);
    line_vertical1 = 240;
    line_vertical2 = 480;
    line_vertical3 = 1440;
    line_vertical4 = 1680;
    line_vertical_mask1 = 480;
    line_vertical_mask2 = 1440;
    line_horizontal1 = 150;
    line_horizontal2 = 600;
    line_horizontal_mask = 360;
    contourAreaMin = 80; //caratterizzazione pallina
    contourAreaMax = 800;
    circularityMin = 0.03;
    maxFrameToLost = 5; //numero di frame che la pallina esce dal rect o non si vede piu
    minDeltaX = minDeltaY = 5; //minimo e massimo spostamento pallina [x,y]
    maxDeltaX = maxDeltaY = 50;
    passingTime = 180; //tempo di scambio

  }else if(videoSelect == 2){ //video 4k Gopro_4k_60fps
    cap.open("Video2.MP4");
    if(!cap.isOpened()){
        cout << "Error opening video stream or file" << endl;
        return -1;
    }

    width=cap.get(CAP_PROP_FRAME_WIDTH);
    height=cap.get(CAP_PROP_FRAME_HEIGHT);
    fps=cap.get(CAP_PROP_FPS);

    cout<<"Frame width: "<<width<<endl;
    cout<<"Frame height: "<<height<<endl;
    cout<<"FPS: "<<fps<<endl;

    rectL = Rect(240*2, 370*2, 290*2, 380*2);
    rectR = Rect(1440*2, 210*2, 300*2, 440*2);
    line_vertical1 = 240;
    line_vertical2 = 530;
    line_vertical3 = 1440;
    line_vertical4 = 1740;
    line_vertical_mask1 = 530;
    line_vertical_mask2 = 1440;
    line_horizontal1 = 210;
    line_horizontal2 = 750;
    line_horizontal_mask = 430;
    contourAreaMin = 240*2; //caratterizzazione pallina
    contourAreaMax = 800*2;
    circularityMin = 0.03;
    maxFrameToLost = 5; //numero di frame che la pallina esce dal rect o non si vede piu
    minDeltaX = minDeltaY = 10; //minimo e massimo spostamento pallina [x,y]
    maxDeltaX = maxDeltaY = 200;
    passingTime = 180; //tempo di scambio

  }else if(videoSelect==3){ //vidoe SX_4k_60fps
    cap.open("Video3.MP4");
    if(!cap.isOpened()){
        cout << "Error opening video stream or file" << endl;
        return -1;
    }

    width=cap.get(CAP_PROP_FRAME_WIDTH);
    height=cap.get(CAP_PROP_FRAME_HEIGHT);
    fps=cap.get(CAP_PROP_FPS);

    cout<<"Frame width: "<<width<<endl;
    cout<<"Frame height: "<<height<<endl;
    cout<<"FPS: "<<fps<<endl;

    rectL = Rect(800*2, 280*2, 400*2, 420*2);
    rectR = Rect(0,0,0,0);
    line_vertical1 = 800;
    line_vertical2 = 1200;
    line_vertical3 = width;
    line_vertical4 = width;
    line_vertical_mask1 = 1200;
    line_vertical_mask2 = width;
    line_horizontal1 = 280;
    line_horizontal2 = 700;
    line_horizontal_mask = height;
    contourAreaMin = 80*2; //caratterizzazione pallina
    contourAreaMax = 800*2;
    circularityMin = 0.03;
    maxFrameToLost = 3; //numero di frame che la pallina esce dal rect o non si vede piu
    minDeltaX = minDeltaY = 10; //minimo e massimo spostamento pallina [x,y]
    maxDeltaX = 200;
    maxDeltaY = 100;
    passingTime = 300; //tempo di scambio
  }

  Ptr<BackgroundSubtractorMOG2> pBackSub;
  pBackSub = createBackgroundSubtractorMOG2();

  while(cap.isOpened()){

    if(play==true){
      cap >> frame1;
      numFrame++; //numero corrente di frame
    }

    resize(frame1, frame_resized, Size(1920, 1080));

    //Eliminiamo parte del rumore
    if(videoSelect==2){
      GaussianBlur(frame1, modif, Size(5,5), 0);
      GaussianBlur(frame1, modif, Size(3,3), 0);
    }else{
      GaussianBlur(frame1, modif, Size(9,9), 0);
    }

    //Copriamo il campo dietro con un rettangolo nero
    if(videoSelect==2 || videoSelect==3){
      rectangle(modif, Point(line_vertical_mask1*2,0), Point(line_vertical_mask2*2, line_horizontal_mask*2), SCALAR_BLACK, -1);
    }else{
      rectangle(modif, Point(line_vertical_mask1,0), Point(line_vertical_mask2, line_horizontal_mask), SCALAR_BLACK, -1);
    }
    rectangle(frame_resized, Point(line_vertical_mask1,0), Point(line_vertical_mask2, line_horizontal_mask), SCALAR_BLACK, -1);

    //Per un riscontro visivo, evidenziamo con delle linee rosse la zona di interesse
    line(frame_resized, Point(line_vertical1,0), Point(line_vertical1,height), SCALAR_RED);
    line(frame_resized, Point(line_vertical2,0), Point(line_vertical2,height), SCALAR_RED);
    line(frame_resized, Point(line_vertical3,0), Point(line_vertical3,height), SCALAR_RED);
    line(frame_resized, Point(line_vertical4,0), Point(line_vertical4,height), SCALAR_RED);
    line(frame_resized, Point(0,line_horizontal1), Point(width,line_horizontal1), SCALAR_RED);
    line(frame_resized, Point(0,line_horizontal2), Point(width,line_horizontal2), SCALAR_RED);

    //Applichiamo la background subtraction
    pBackSub->apply(modif, fgMask);

    //Smoothing e Thresholding
    GaussianBlur(fgMask, fgMask, Size(7,7), 0);
    threshold(fgMask, fgMask, 60, 255, THRESH_BINARY);

    //Apertura, per rimuovere ulteriori componenti di rumore. Chiusura "aggressiva" per uniformare le figure in movimento
    if(videoSelect==2){
      erode(fgMask, fgMask, getStructuringElement(MORPH_ELLIPSE, Size(6,6)));
      dilate(fgMask, fgMask, getStructuringElement(MORPH_ELLIPSE, Size(15,15)));

      dilate(fgMask, fgMask, getStructuringElement(MORPH_ELLIPSE, Size(30,30)));
      erode(fgMask, fgMask, getStructuringElement(MORPH_ELLIPSE, Size(30,30)));
    }else{
      erode(fgMask, fgMask, getStructuringElement(MORPH_ELLIPSE, Size(4,4)));
      dilate(fgMask, fgMask, getStructuringElement(MORPH_ELLIPSE, Size(9,9)));

      dilate(fgMask, fgMask, getStructuringElement(MORPH_ELLIPSE, Size(30,30)));
      erode(fgMask, fgMask, getStructuringElement(MORPH_ELLIPSE, Size(30,30)));
    }

    //Ricorriamo ai contorni per identificare la pallina
    findContours(fgMask, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

    //Funzione che permette di localizzare la pallina nelle zone di interesse
    blobCenter(contours);

    /*Scelgo tre punti in cui memorizzo la posizione della pallina, da cui poi possiamo calcolare la velocità media nell'area di interesse.
     Aumentando la dimensione di questo array aumenta l'accuratezza del calcolo della forza, ma aumentano anche i casi di miss, in quanto
     dovrò avere un maggior numero di posizioni in cui individuo la pallina*/
    Point recordSX[2];
    Point recordDX[2];
    // Ho bisogno dei tempi relativi tra le posizioni individuate
    double timesSX[2];
    double timesDX[2];

    //Tutti i dati soo forniti dalle funzioni ballLocation

    // Riempio le posizioni in blocchi da 3
    vector<Point>::const_iterator it1;
    vector<int>::const_iterator it2;

    //Appena sono disponibili 3 posizioni nella regione di interesse per una traiettoria (ballInfoSX==TRUE), e i dati non sono ancora
    if(ballInfoSX == true && data_readySX==false){
      data_readySX=true;
      int j=0;
      for(it1=possibleBallInfoSX.begin(); it1!=possibleBallInfoSX.end(); it1++) {
        recordSX[j]=*it1;
        j++;
      }
      int k=0;
      for(it2=framePassedSX.begin(); it2!=framePassedSX.end(); it2++) {
        timesSX[k]=*it2;
        timesSX[k]=timesSX[k] / fps;
        k++;
      }
      ballInfoSX = false;
    }

    if(ballInfoDX == true && data_readyDX==false){
      data_readyDX=true;
      int l=0;
      for(it1=possibleBallInfoDX.begin(); it1!=possibleBallInfoDX.end(); it1++) {
        recordDX[l]=*it1;
        l++;
      }
      int q=0;
      for(it2=framePassedDX.begin(); it2!=framePassedDX.end(); it2++) {
        timesDX[q]=*it2;
        timesDX[q]=timesDX[q] / fps;
        q++;
      }
      ballInfoDX = false;
    }

    // Scommentare per vedere cosa succede all'interno degli array posizioni e tempi
    //cout << recordSX[0] << " " << recordSX[1] << " " << recordSX[2] << " - " << timesSX[1] << " " << timesSX[2] << endl;

    // REGIONE DI INTERESSE DI SINISTRA. Se i dati sono pronti, posso calcolare la velocità della traiettoria

    if(data_readySX==true){
      // Se i punti sono crescenti lungo x, la palla va da sinistra verso destra, quindi aggiorno la velocità di partenza. Faccio una media delle velocità tra il punto 1 e 2 e di quella tra il punto 2 e 3
      if(recordSX[1].x>recordSX[0].x && recordSX[2].x>recordSX[1].x){
        part_speedSX=sqrt(((recordSX[1].x - recordSX[0].x)*(recordSX[1].x - recordSX[0].x))+(recordSX[1].y - recordSX[0].y)*(recordSX[1].y - recordSX[0].y)) / timesSX[1];
        part_speedSX=part_speedSX + (sqrt(((recordSX[2].x - recordSX[1].x)*(recordSX[2].x - recordSX[1].x))+(recordSX[2].y - recordSX[1].y)*(recordSX[2].y - recordSX[1].y)) / timesSX[2]);
        part_speedSX=part_speedSX/2;
        // Se non è arrivata la pallina nei frame precedenti o non è stata trovata, considero il colpo come una battuta (velocità iniziale=0)
        if(arrivedSX==false){
          // Forza = (massa_pallina=0.058kg * Velocità finale) / tempo_di_contatto=0.005s
          applied_forceSX=(0.058 * part_speedSX) / 0.005;
          //Ci sono dei casi in cui la pallina rotola; Quindi non andrò a mostrare i casi in cui la forza è sotto un valore realistico per una battuta
          if(applied_forceSX>8000){
            cout << "BATTUTA da SINISTRA con FORZA: " << applied_forceSX <<" kg * pixel/s^2 " << endl;
          }
        }else{ // Ho un valore di velocità di arrivo, da tenere in considerazione per il calcolo della forza
          applied_forceSX=(0.058 * (part_speedSX + arr_speedSX)) / 0.005;
          arrivedSX=false;
          cout << "RISPOSTA da SINISTRA con FORZA: " << applied_forceSX << " kg * pixel/s^2" << endl;
        }
        //Scommentare per vedere con che velocità parte la pallina
        // cout << "Velocità di partenza da Sinistra:" << part_speedSX << "pixel/secondo" << endl;

        //Se la velocità è sotto un certo valore realistico, stampiamo che si tratta di un errore di misurazione
        if(part_speedSX < 50){
          cout << "Errore misurazione" << endl;
        }
      }
      // Se i punti sono decrescenti, la pallina sta arrivando, quindi aggiorno la velocità di arrivo e setto che la pallina è arrivata (arrived=true)
      if(recordSX[1].x<recordSX[0].x && recordSX[2].x<recordSX[1].x){
        arr_speedSX=sqrt(((recordSX[0].x - recordSX[1].x)*(recordSX[0].x - recordSX[1].x))+(recordSX[0].y - recordSX[1].y)*(recordSX[0].y - recordSX[1].y)) / timesSX[1];
        arr_speedSX=arr_speedSX + (sqrt(((recordSX[1].x - recordSX[2].x)*(recordSX[1].x - recordSX[2].x))+(recordSX[1].y - recordSX[2].y)*(recordSX[1].y - recordSX[2].y)) / timesSX[2]);
        arr_speedSX=arr_speedSX/2;
        arrivedSX=true;
      }
      // La forza è stata calcolata; pongo nuovamente l'algoritmo in ascolto
      data_readySX=false;
    }

    //Se aspetto più di un certo valore, è molto probabile che il gioco si sia fermato, quindi non devo più considerare la velocità con cui è arrivata la pallina
    if(arrivedSX == true){
      arrCountSX++;
    }
    if(arrCountSX > passingTime){
      arrivedSX = false;
      arrCountSX = 0;
    }

    //REGIONE DI INTERESSE DI DESTRA -> analogo alla parte sinistra, chiaramente in modo specchiato
    if(data_readyDX==true){
      // Se i punti sono decrescenti lungo x, la palla va da destra verso sinistra, quindi aggiorno la velocità di partenza
      if(recordDX[1].x<recordDX[0].x && recordDX[2].x<recordDX[1].x){
        part_speedDX=sqrt(((recordDX[1].x - recordDX[0].x)*(recordDX[1].x - recordDX[0].x))+(recordDX[1].y - recordDX[0].y)*(recordDX[1].y - recordDX[0].y)) / timesDX[1];
        part_speedDX=part_speedDX + (sqrt(((recordDX[2].x - recordDX[1].x)*(recordDX[2].x - recordDX[1].x))+(recordDX[2].y - recordDX[1].y)*(recordDX[2].y - recordDX[1].y)) / timesDX[2]);
        part_speedDX=part_speedDX/2;
        // Se non è arrivata la pallina nei frame precedenti o non è stata trovata, considero il colpo come una battuta
        if(arrivedDX==false){
          // Forza = (massa_pallina=0.058kg * Velocità finale) / tempo_di_contatto=0.005s
          applied_forceDX=(0.058 * part_speedDX) / 0.005;
          if(applied_forceDX>8000){
            cout << "BATTUTA da DESTRA con FORZA: " << applied_forceDX <<" kg * pixel/s^2 " << endl;
          }
        }else{ // Ho un valore di velocità di arrivo, da tenere in considerazione per il calcolo della forza
          applied_forceDX=(0.058 * (part_speedDX + arr_speedDX)) / 0.005;
          arrivedDX=false;
          cout << "RISPOSTA da DESTRA con FORZA: " << applied_forceDX << " kg * pixel/s^2" << endl;
        }
        if(part_speedDX<50){
          cout << "Errore misurazione " << endl;
        }
      }
      // Se i punti sono crescenti, la pallina sta arrivando, quindi aggiorno la velocità di arrivo
      if(recordDX[0].x<recordDX[1].x && recordDX[1].x<recordDX[2].x){
        arr_speedDX=sqrt(((recordDX[0].x - recordDX[1].x)*(recordDX[0].x - recordDX[1].x))+(recordDX[0].y - recordDX[1].y)*(recordDX[0].y - recordDX[1].y)) / timesDX[1];
        arr_speedDX=arr_speedDX + (sqrt(((recordDX[1].x - recordDX[2].x)*(recordDX[1].x - recordDX[2].x))+(recordDX[1].y - recordDX[2].y)*(recordDX[1].y - recordDX[2].y)) / timesDX[2]);
        arr_speedDX=arr_speedDX/2;
        arrivedDX = true;
      }
      data_readyDX = false;
    }

    if(arrivedDX == true){
      arrCountDX++;
    }
    if(arrCountDX > passingTime){
      arrivedDX = false;
      arrCountDX = 0;
    }

    // libero il vettore contours per evitare un sovraccarico di memoria
    contours.clear();

    imshow("Mask", fgMask);
    imshow("Processed Frame", frame_resized);
    resizeWindow("Mask", fgMask.cols/4, fgMask.rows/4);
    resizeWindow("Processed Frame", frame_resized.cols/2, frame_resized.rows/2);

    char key=waitKey(5);
    if(key=='q'){
      break;
    }else if(key=='p'){
      play=!play;
    }
  } //end while

  contours.clear();
  possibleBallInfoSX.clear();
  framePassedSX.clear();

  return 0;
}
  /*////////////////////////////////////////////////////////////////////////////
  funzione che trova il centro di un oggetto e ne distingue la forma
  ////////////////////////////////////////////////////////////////////////////*/
void blobCenter(vector<vector<Point>> &points){
  int objectsDX = 0;
  int objectsSX = 0;
  Point p;
  vector<vector<Point> >::iterator vit;
  for(vit=points.begin(); vit!=points.end(); ++vit){ //se un frame non ha informazione qui non entra
    double arclentgh = arcLength(*vit, true);
    double circularity= 4*CV_PI*contourArea(*vit)/(arclentgh * arclentgh);
    if(contourArea(*vit)<contourAreaMax && contourArea(*vit)>contourAreaMin && circularity>circularityMin){
      RotatedRect candidate = fitEllipse(*vit);
      p=candidate.center;
      if(videoSelect==1){
        if(p.inside(rectL)==true){
          rectangle(frame_resized, Point(p.x-5, p.y-5), Point(p.x+5,p.y+5), SCALAR_BLUE, 2, 8, 0);
          objectsSX++;
          if(objectsSX == 1){ //esegue solo se trova un oggetto di dimensioni pallina e se ce n'è solo uno nei due rettangoli
            ballFrameSX++;
            ballLocationSX(p);
          }
        }
        if(p.inside(rectR)==true){ //eseguo solo se ce qualcosa nei rettangoli
          rectangle(frame_resized, Point(p.x-5, p.y-5), Point(p.x+5,p.y+5), SCALAR_BLUE, 2, 8, 0);
          objectsDX++;
          if(objectsDX == 1){ //esegue solo se trova un oggetto di dimensioni pallina e se ce n'è solo uno nei due rettangoli
            ballFrameDX++;
            ballLocationDX(p);
          }
        }
      }
      if(videoSelect==2 || videoSelect==3){
        if(p.inside(rectL)==true){
          rectangle(frame_resized, Point(p.x/2-5, p.y/2-5), Point(p.x/2+5,p.y/2+5), SCALAR_BLUE, 2, 8, 0);
          objectsSX++;
          if(objectsSX == 1){ //esegue solo se trova un oggetto di dimensioni pallina e se ce n'è solo uno nei due rettangoli
          ballFrameSX++;
          ballLocationSX(p);
          }
        }
        if(p.inside(rectR)==true){ //eseguo solo se ce qualcosa nei rettangoli
          rectangle(frame_resized, Point(p.x/2-5, p.y/2-5), Point(p.x/2+5,p.y/2+5), SCALAR_BLUE, 2, 8, 0);
          objectsDX++;
          if(objectsDX == 1){ //esegue solo se trova un oggetto di dimensioni pallina e se ce n'è solo uno nei due rettangoli
            ballFrameDX++;
            ballLocationDX(p);
          }
        }
      }
    }
  }
}

  /*////////////////////////////////////////////////////////////////////////////
  le due funzioni sottostanti vengono chiamate solo se viene trovato un oggetto
  di dimensioni circa di una pallina nelle regioni di interesse relative
  ////////////////////////////////////////////////////////////////////////////*/
  void ballLocationSX(Point &ball){ //introduco controllo perdita della pallina
    if(numFrame-ballFrameSX < maxFrameToLost){ //massimo numero di volte che perdo la pallina tra due misure
      ballValueSX++;
      if(ballValueSX >= 2){
        if((fabs(possibleBallInfoSX.back().x - ball.x) < minDeltaX && fabs(possibleBallInfoSX.back().y - ball.y) < minDeltaY) ||
          (fabs(possibleBallInfoSX.back().x - ball.x) > maxDeltaX && fabs(possibleBallInfoSX.back().y - ball.y) > maxDeltaY)){ //un uomo cammina a 1m/s //guarda il valore precedente
          //possibleBallInfo.pop_back();//cancella il precedente non quello da inserire
          notballSX = true;
        }else if(ballValueSX == 3){
          ballInfoSX=true;
        }
      }
      if(notballSX == false){ //se il dato corrente e valido aggiungi altrimenti no e decrementa contatore
        possibleBallInfoSX.push_back(ball);
        framePassedSX.push_back(numFrame-ballFrameSX+1);
      }else{
        notballSX = false;
        ballValueSX--;
      }
    }else{
      ballValueSX = 0;
      possibleBallInfoSX.clear();
      framePassedSX.clear();
    }
    ballFrameSX = numFrame;
  }

  void ballLocationDX(Point &ball){ //introduco controllo perdita della pallina
    if(numFrame-ballFrameDX < maxFrameToLost){ //massimo numero di volte che perdo la pallina tra due misure
      ballValueDX++;
      if(ballValueDX >= 2){
        if((fabs(possibleBallInfoDX.back().x - ball.x) < minDeltaX && fabs(possibleBallInfoDX.back().y - ball.y) < minDeltaY) ||
          (fabs(possibleBallInfoDX.back().x - ball.x) > maxDeltaX && fabs(possibleBallInfoDX.back().y - ball.y) > maxDeltaY)){ //un uomo cammina a 1m/s //guarda il valore precedente
          //possibleBallInfo.pop_back();//cancella il precedente non quello da inserire
          notballDX = true;
        }else if (ballValueDX == 3){
          ballInfoDX=true;
        }
      }
      if(notballDX == false){ //se il dato corrente e valido aggiungi altrimenti no e decrementa contatore
        possibleBallInfoDX.push_back(ball);
        framePassedDX.push_back(numFrame-ballFrameDX+1);
      }else{
        notballDX = false;
        ballValueDX--;
      }
    }else{
      ballValueDX = 0;
      possibleBallInfoDX.clear();
      framePassedDX.clear();
    }
    ballFrameDX = numFrame;
  }
