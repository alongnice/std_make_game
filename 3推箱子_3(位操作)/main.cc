#include<iostream>
#include<fstream>
#include<algorithm>
using namespace std;

//函数原型
void readFile( char** buffer, int* size, const char* filename );

//二位数组类
template< class T > class Array2D{
public:
    //构造函数
    Array2D():mArray(0){}
    //析构函数
    ~Array2D(){
        delete[] mArray;
        mArray = 0;
    }
    //以下几个函数的目的都是一维二维的转化
    void setSize( int size0, int size1 ){
        mSize0 = size0;
        mSize1 = size1;
        mArray = new T[ size0 * size1 ];
    }
    T& operator()( int index0, int index1 ){
        return mArray[ index1 * mSize0 + index0 ];
    }
    T& operator()( int index0, int index1 ) const {
        return mArray[ index1 * mSize0 + index0];
    }

private:
    T* mArray;
    int mSize0;
    int mSize1;
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

        OBJ_GOAL_FLAG = ( 1 << 7 ), //终点标记位
    };
    void setSize( const char* stageData, int size );
    int mWidth;
    int mHeigth;
    Array2D< unsigned char > mObjects;  //由于位运算的需要，这里使用unsigned char类型
};

int main( int argc, char** argv ){
    const char* filename = "stageData.txt";
    if ( argc >= 2 )
        filename = argv[1];

    char* stageData;
    int fileSize;
    readFile( &stageData, &fileSize, filename );
    if ( !stageData ){
        cout<<"文件读取失败"<<endl;
        return 1;
    }

    State* state = new State( stageData, fileSize );

    //主循环
    while ( true ){
        //首先绘制
        state->draw();
        //通关检测
        if ( state->hasCleared() ){
            break;
        }
        //提示信息
        cout<< "a:<-\td:->\tw:^\ts:v"<<endl;
        char input;
        cin >> input;;
        //刷新
        state->update(input);
    }
    //游戏过关
    cout << "游戏过关" << endl;
    //析构
    delete[] stageData;
    stageData = 0;
    //无限循环（ctrl+c quit）
    while ( true ){;}
    return 0;
}

//----------function  definition--------------
void readFile( char** buffer, int* size, const char* filename ){
    ifstream in( filename );
    if ( !in ){
        *buffer = 0;
        *size = 0;
    }else{
        in.seekg( 0,in. end );
        *size = static_cast< int >( in.tellg() );
        in.seekg( 0, in.beg );
        *buffer = new char[ *size ];
        in.read( *buffer, *size);
    }
}

State::State(const char* stageData, int size){
    //确保容量
    setSize( stageData, size );
    //确保空间
    mObjects.setSize( mWidth, mHeigth );
    //预设初始值
    for (int y = 0; y < mHeigth; ++y){
        for (int x = 0; x < mWidth; ++x){
            mObjects( x, y ) = OBJ_WALL;    //多余部分设置为墙
        }
    }
    int x = 0;
    int y = 0;
    for (int i = 0; i < size; ++i){
        unsigned char t;
        switch ( stageData[ i ] )
        {
            case '#': t = OBJ_WALL;  break;
            case ' ': t = OBJ_SPACE;  break;
            case 'o': t = OBJ_BLOCK;  break;
            case 'O': t = OBJ_BLOCK | OBJ_GOAL_FLAG;  break;
            case '.': t = OBJ_SPACE | OBJ_GOAL_FLAG;  break;
            case 'p': t = OBJ_MAN;  break;
            case 'P': t = OBJ_MAN | OBJ_GOAL_FLAG;  break;
            case '\n': x = 0; ++y; t = OBJ_UNKNOWN;  break;
            default:  t = OBJ_UNKNOWN;  break;
        }
        if ( t != OBJ_UNKNOWN ){
            mObjects( x,y ) = t; //写入
            ++x;
        }
    }
}

void State::setSize( const char* stageData, int size ){
    mWidth = mHeigth = 0;   //初始化
    //当前位置
    int x = 0;
    int y = 0;
    for(int i = 0; i< size; ++i){
        switch ( stageData[i] ){
            case '#': case ' ': case 'o':case 'O':
            case '.': case 'p': case 'P':
                ++x;
                break;
            case '\n':
                ++y;
                //更新最大值
                mWidth = max( mWidth, x );
                mHeigth = max( mHeigth, y );
                x = 0;
                break;
        }
    }
}

//绘画图像
void State::draw() const{
    for ( int y = 0; y < mHeigth; ++y){
        for ( int x = 0; x < mWidth; ++x ){
            switch ( mObjects(x,y) ){
                case ( OBJ_SPACE | OBJ_GOAL_FLAG ): cout<<"."; break;
                case ( OBJ_WALL | OBJ_GOAL_FLAG ) : cout<<"#"; break;
                case ( OBJ_BLOCK | OBJ_GOAL_FLAG ): cout<<"O"; break;
                case ( OBJ_MAN | OBJ_GOAL_FLAG )  : cout<<"P"; break;
                case OBJ_SPACE                    : cout<<" "; break;
                case OBJ_WALL                     : cout<<"#"; break;
                case OBJ_BLOCK                    : cout<<"o"; break;
                case OBJ_MAN                      : cout<<"p"; break;
            }
        }
        cout << endl;
    }
}

void State::update( char input ){
    //移动量变换
    int dx = 0;
    int dy = 0;
    switch ( input ){
        case 'a': dx = -1; break;   //👈
        case 'd': dx =  1; break;   //👉
        case 'w': dy = -1; break;   //👆
        case 's': dy =  1; break;   //👇
    }
    //使用较短的变量名。这里试着使用了C++中的引用，注意添加const关键字保护内容
    const int& w = mWidth;
    const int& h = mHeigth;
    Array2D< unsigned char >& o = mObjects;
    //查找小人的坐标
    int x,y;
    x = y = -1;
    bool found = false;
    for ( y = 0; y < h; ++y){
        for ( x = 0; x < w; ++x){
            if ( ( o( x, y) & ~OBJ_GOAL_FLAG ) == OBJ_MAN ) {
                found = true;
                break;
            }
        }
        if ( found )
            break;
    }
    //移动
    //移动后的坐标
    int tx = x + dx;
    int ty = y + dy;
    //检测坐标的极端值
    if( tx < 0 || ty < 0 || tx >= w || ty >= h ){
        return ;
    }
    //A.如果该方向上是空白或者重点。则小人移动
    if ( ( o( tx,ty ) & ~OBJ_GOAL_FLAG ) == OBJ_SPACE ){
        o( tx,ty ) = ( o(tx,ty) & OBJ_GOAL_FLAG ) | OBJ_MAN;
        //保存终点标志位
        o ( x,y ) = ( o( x, y ) & OBJ_GOAL_FLAG ) | OBJ_SPACE;
    }else if ( o(tx, ty) == OBJ_BLOCK ){
        //检测同方向上的下下个格子是否是合理值范围
        int tx2 = tx + dx;
        int ty2 = ty + dy;
        if ( tx2 < 0 || ty2 < 0 || tx2 >= w || ty >= h ){
            return ;
        }
        if ( ( o( tx2, ty2) & ~OBJ_GOAL_FLAG ) == OBJ_SPACE ){
            //按顺序替换
            o ( tx2, ty2 ) = ( o( tx2, ty2) & OBJ_GOAL_FLAG ) | OBJ_BLOCK;
            o ( tx, ty ) = ( o( tx, ty) & OBJ_GOAL_FLAG )| OBJ_BLOCK;
            o ( x, y ) = ( o( x, y) & OBJ_GOAL_FLAG ) | OBJ_SPACE;
        }
    }
}

//只要还存在一个goalFlag的值 都不算通关
bool State::hasCleared()  const{
    for ( int y = 0; y < mHeigth; ++y ){
        for ( int x = 0; x < mWidth; ++x ){
            if ( mObjects(x, y) == OBJ_BLOCK ){
                return false;
            }
        }
    }
    return true;
}

