#include <Wire.h>

#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"

const byte MLX90640_address = 0x33; //Default 7-bit unshifted address of the MLX90640

#define TA_SHIFT 8 //Default shift for MLX90640 in open air
int count;
static float mlx90640To[768];
paramsMLX90640 mlx90640;
//void get_frame(void);
//float get_bitn();
void setup()
{
  Wire.begin();
  Wire.setClock(400000); //Increase I2C clock speed to 400kHz

  Serial.begin(9600);
  while (!Serial); //Wait for user to open terminal
  Serial.println("MLX90640 IR Array Example");

  if (isConnected() == false)
  {
    Serial.println("MLX90640 not detected at default I2C address. Please check wiring. Freezing.");
    while (1);
  }
  Serial.println("MLX90640 online!");

  //Get device parameters - We only have to do this once
  int status;
  uint16_t eeMLX90640[832];
  status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);
  if (status != 0)
    Serial.println("Failed to load system parameters");

  status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
  if (status != 0)
    Serial.println("Parameter extraction failed");

  //Once params are extracted, we can release eeMLX90640 array
}

void loop()
{
  // get_frame();
  long stopTime = millis();
  // for (int x = 0 ; x < 768 ; x++)
  // {
   // if(x%23==0)Serial.println();
   // Serial.print("Pixel ");
   // Serial.print(x);
   // Serial.print(": ");
   // Serial.print(mlx90640To[x], 2);
   // Serial.print("C");
   // Serial.println();
    // if(x % 24 == 0) Serial.println();
    // Serial.print(mlx90640To[x], 2);
    // Serial.print(",");
  // }
   // Serial.println("");
  //delay(1000);
  
  float tempature = get_bitn();
  Serial.println(tempature); 
  Serial.println(count);
  if(count>5)
   Serial.println("YES");
   else
   Serial.println("NO");
  count=0;
}

//Returns true if the MLX90640 is detected on the I2C bus
boolean isConnected()
{
  Wire.beginTransmission((uint8_t)MLX90640_address);
  if (Wire.endTransmission() != 0)
    return (false); //Sensor did not ACK
  return (true);
}

void get_frame(void)
{
  for (byte x = 0 ; x < 2 ; x++) //Read both subpages
  {
    uint16_t mlx90640Frame[834];
    int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);
    if (status < 0)
    {
      Serial.print("GetFrame Error: ");
      Serial.println(status);
    }

    float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
    float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);

    float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
    float emissivity = 0.95;

    MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, mlx90640To);
  }
}
//制作二维灰度值图像
float get_bitn()
{
  float Tri_FPS_mlx90640To[768];
  float max_tempature=0;
  for(int i=0;i<3;i++)
  {
    get_frame();
    for(int j=0;j<768;j++)
    { 
      if(i==2)
      { 
        Tri_FPS_mlx90640To[j]/=2.0;
        Tri_FPS_mlx90640To[j]=mlx90640To[j]-Tri_FPS_mlx90640To[j];
        if(Tri_FPS_mlx90640To[j]>1.5)
        {
          count++;
          if(mlx90640To[j]>max_tempature)
          {
            max_tempature=mlx90640To[j];
          }
          Tri_FPS_mlx90640To[j]=1;
        }
        else
        {
          Tri_FPS_mlx90640To[j]=0;
        }
      }
      else
      {
        Tri_FPS_mlx90640To[j]+=mlx90640To[j];
      } 
    }
  }
  //四邻域标记
  return max_tempature;
}

/* void make(float Tri_FPS_mlx90640To)
{
  
  float pixels[24][32];
  int st_x=0,st_y=0;
  int dx[4]={-1,0,1,0};
  int dy[4]={0,-1,0,1};
  bool v[24][32];
  int flag=0;
  struct point {
    int x;
    int y;
    int st;
  };
  queue<point> r;
  
  for(int i=0;i<32;i++)
  {
    for(int j=0;j<24;j++)
    {
      if(flag==0&&Tri_FPS_mlx90640To[i*24+j])
      {
        st_x=i;
        st_y=j;
        flag=1;
      }
      pixels[i][j]=Tri_FPS_mlx90640To[i*24+j];
    }
  }
  
  point start;
  start.x = st_x;
  start.y = st_y;
  start.st = 1;
  r.push(start);
  v[st_x][st_y]=true;
  while(!r.empty)
  {
    for(int k=0;k<4;k++)
    {
      int x,y;
      x=r.front().x+dx[k];
      y=r.front().y+dy[k];
      if(x>=0&&x<24&&y>=0&&y<32&&pixels[x][y]==1&&v[x][y]==false)
      {
        point temp;
        temp.x=x;
        temp.y=y;
        temp.st=r.front().st;
        v[x][y]=true;
        r.push(temp);
      }
    }
    r.pop();
  }
} */
