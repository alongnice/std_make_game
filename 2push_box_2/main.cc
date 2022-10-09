#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;

//函数声明
void readFile( char** buffer, int* size, const char* filename );

//二维数组类


template< class T > class Array2D{
public:
    Array2D() : mArray( 0 ){}
    ~Array2D(){
        delete[] mArray;
        mArray = 0;     //安全起见指针设置为0
    }
    void setSize(int size0,int size1){
        msize0 = size0;        
        msize1 = size1;        
        mArray = new T[size0 * size1];
    }
	T& operator()( int index0, int index1 ){
        return mArray[index1 * msize0 + index0];
	}
    const T& operator()(int index0,int index1) const{
        return mArray[index1 * msize0 + index0];
    }

private:
    T* mArray;
    int msize0;
    int msize1;
};

//状态类
class State{
public:
    State( const char* stageData, int size );
    void update( char input );
    void draw() const;
    bool hasCleared() const;
private:
    enum Object{
        OBJ_SPACE,
        OBJ_WALL,
        OBJ_BLOCK,
        OBJ_MAN,

        OBJ_UNKNOWN,
    };
    void setSize( const char* stageData, int size );

    int mWidth;
    int mHeight;
    Array2D< Object > mObjects;
    Array2D< bool > mGoalFlags;
};

int main( int argc, char** argv ){
    const char* filename = "stageData.txt";     //默认地图存储文件
    if ( argc >= 2 )
        filename = argv[ 1 ];                     //选定地图存储文件
    char* stageData;                            //存放地图数据信息
    int fileSize;                               //文件大小
    readFile( &stageData, &fileSize, filename ); //读取文件到stagedata
    if( !stageData ){
        cout << "stage file could not be read." << endl;
        return 1;
    }
    State* state = new State( stageData, fileSize );

    //主循环
    while ( true ){
      cout << "\33[2J\33[1;1H";
        state->draw();                          //通关检测
        if( state->hasCleared() )
            break;                              //游戏结束
        //提示操作内容
        cout<< "a:<-\td:->\tw:^\ts:v"<<endl;    //操作说明
        char input;
        cin >> input;
        //刷新
        state->update( input );
    }
    //通关提示信息
    cout << "congratulation's! you won." << endl;
    //析构
    delete[] stageData;
    stageData = 0;
    //无线循环（ctrl+c退出）
    while(true){;}
    return 0;
}

//-------------------------下面是函数定义---------------------------

/* @brief 读取文件
 @param buffer 返回参数，最终文件内容的内存块名称（二级指针）
 @param size 返回参数，文件的大小
 @param filename 传入参数，文件名称
*/
void readFile( char** buffer, int* size, const char* filename ){
    ifstream in( filename );
    if ( !in ){
        *buffer = 0;
        *size = 0;
    }else{
        in.seekg(0,ifstream::end);           //漂移到文件结尾
        *size = static_cast<int>(in.tellg());//返回文件的大小
        in.seekg(0,ifstream::beg);           //漂移到文件开头
        *buffer = new char[*size];           //读取内容，使用指定大小
        in.read(*buffer, *size);             //读取进入buffer缓存中
    }
}

State::State(const char* stageData,int size){
    //确保容量
    setSize(stageData,size);
    //确保空间
    mObjects.setSize(mWidth,mHeight);
    mGoalFlags.setSize(mWidth,mHeight);
    //预设初始值
    for(int y=0;y<mHeight;++y){
        for(int x=0;x<mWidth;++x){
            mObjects(x,y) = OBJ_WALL;       //多余部分设置为墙壁
            mGoalFlags(x,y) = false;        //非终点
        }
    }
    int x = 0;
    int y = 0;
    for ( int i = 0; i < size; ++i ){
        Object t;
        bool goalFlag = false;
        switch ( stageData[ i ] ){
            case '#': t = OBJ_WALL ; break;
            case ' ': t = OBJ_SPACE ; break;
            case 'o': t = OBJ_BLOCK ; break;
            case 'O': t=OBJ_BLOCK; goalFlag = true ; break;
            case '.': t=OBJ_SPACE; goalFlag = true ; break;
            case 'p': t=OBJ_MAN ; break;
            case 'P': t=OBJ_MAN ; goalFlag = true;   break;
            case '\n': x=0;++y;t = OBJ_UNKNOWN ; break;
            default: t = OBJ_UNKNOWN;break;
        }
        if(t!=OBJ_UNKNOWN){             //如果if处理的意义是未定义元素值就跳过
            mObjects(x,y) = t;          //写入
            mGoalFlags(x,y) = goalFlag; //重点信息
            ++x;
        }
    }
}

void State::setSize(const char* stageData,int size){
    mWidth = mHeight = 0;       //进行初始化
    //当前位置
    int x=0;
    int y=0;
    for(int i =0;i<size;++i){
        switch(stageData[i]){
            case '#':
            case ' ':
            case 'o':
            case 'O':
            case '.':
            case 'p':
            case 'P':
                ++x;
                break;
            case '\n':
                ++y;
                //更新最大值
                mWidth = max(mWidth,x);
                mHeight = max(mHeight,y);
                x=0;
                break;
        }
    }
}

void State::draw() const{
    for(int y=0;y<mHeight;++y){
        for(int x=0;x<mWidth;++x){
            Object o = mObjects(x,y);
            bool goalFlag = mGoalFlags(x,y);
            if(goalFlag){
                switch(o){
                    case OBJ_SPACE  : cout << '.'; break;
                    case OBJ_WALL   : cout << '#'; break;
                    case OBJ_BLOCK  : cout << 'O'; break;
                    case OBJ_MAN    : cout << 'P'; break;
                }
            }else{
                switch(o){
                    case OBJ_SPACE  : cout << ' '; break;
                    case OBJ_WALL   : cout << '#'; break;
                    case OBJ_BLOCK  : cout << 'o'; break;
                    case OBJ_MAN    : cout << 'p'; break;
                }
            }
        }
        cout<<endl;
    }
}


void State::update(char input){
    //移动量变化
    int dx = 0;
    int dy = 0;
    switch(input){
        case 'a':dx = -1; break;        //👈
        case 'd':dx =  1; break;        //👉
        case 'w':dy = -1; break;        //👆
        case 's':dy =  1; break;        //👇
    }
    //使用较短的变量名
    int w = mWidth;
    int h = mHeight;
    Array2D<Object>& o = mObjects;
    //查找小人坐标
    int x,y;
    x=y=-1;             //危险值
    bool found = false;
    for(y = 0;y<mHeight;++y){
        for(x=0;x<mWidth;++x){
            if(o(x,y)==OBJ_MAN){
                found = true;
                break;
            }
        }
        if(found)
            break;
    }
    //移动
    int tx = x+dx;
    int ty = y+dy;

    //极端值判断
    if(tx<0||ty<0||tx>=w||ty>=h)
        return;
    if(o(tx,ty) == OBJ_SPACE){
    //A. 如果该方向上的是空白或者是重点。则小人移动
        o(tx,ty) =OBJ_MAN;
        o(x,y) = OBJ_SPACE;
        
    }else if(o(tx,ty) == OBJ_BLOCK){
    //B. 如果该方向上是箱子。并且该方向的下个各自是空白或者重点则允许移动
        //检测同方向上的下下个格子是否是合理值的范围
        int tx2 = tx+dx;
        int ty2 = ty+dy;
        if(tx2<0||ty2<0||tx>=w||ty>=h){
            return ;
        }
        if(o(tx2,ty2)==OBJ_SPACE){
            //按顺序替换
            o(tx2,ty2) = OBJ_BLOCK;
            o(tx,ty) = OBJ_MAN;
            o(x,y) = OBJ_SPACE;
        }
    }
}

//只要还有存在的goalflag的值就不能判定通关
bool State::hasCleared() const {
    for( int y = 0; y < mHeight; ++y){
        for( int x = 0;x < mWidth; ++x){
            if( mObjects(x,y) == OBJ_BLOCK){
                if( mGoalFlags(x,y) == false){
                    return false;
                }
            }
        }
    }
    return true;
}



