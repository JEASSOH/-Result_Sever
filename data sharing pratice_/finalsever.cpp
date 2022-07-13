//서버
//사실 금방 구현했는데 중간에 무언가(?)의 실수로 그냥 다 갈아엎어버렸다 
//무언가의 실수는 정말 간단하고 사소한 실수인데 함수를 잘못적고 왜 안되냐고 내 자신한테 따지고 있었다.
// 
// 뮤텍스를 이용하고자 했는데 따로 다시 함수정의하기 귀찮고 lock과 unlock을 편히 사용하기위해 pthread를 사용했다
// 그래서 만약 pthread를 따로 설치하지 않는다면 코드 실행이 불가능하다 참고했다면 좋겠다.
// 코드 구현은 주로 pthread의 뮤텍스를 사용하여 lock과 unlock을 사용했다.  클라이언트가 서버에 접속할때 변수를 하나씩 만들어 거기에 주소를 넣고 그대로 쓰레드를 만드는 형식으로 구현했다.
// 
//이런식으로 구현했는데 괜찮은건지 아님 이게 원래 방법인지는 잘 모르겠다. 다만 서버의 share값이 정상적으로 바뀌고 클라이언트를 한번씩 거칠때마다 share+10값을 받고 share+10값을 보낸다 클라이언트에서
// 잘 받고있는지 확인이 가능했다
//pthread를 굳이 사용하지 않고 처음에 받은 코드에다가 내가 처음에 구현했던 코드대로 추가해 사용해도 잘 적용이 될거 같은 느낌이 들지만 시간이 없어 테스트는 못했기에 확실하진 않다.
// 
// 
//그리고 사실 주고받는 값을 이런식으로 처리해도 괜찮은지도 잘 모르겠다. 문제도 100퍼센트 이해하지 못한        게 조금 크다
// 예를들어 sever에있는 share 값이 처음에 0으로 초기화가 되있고 이것이 client에게 주소값을 받은뒤 share을 client에게 보내주고 share값을 받은 client는 sever에서 받은 share에 +10을 해서 보내주면
//sever에 있는 share값이 20으로 변한다는것이 내가 이해한 바이다.

#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#define _CRT_SECURE_NO_WARNINGS
#define SERVERPORT 9000
#define BUFSIZE    512
#define NUM_OF_GATE 256
CRITICAL_SECTION cs;

pthread_mutex_t mutx;


int clnt_cnt = 0;
int clnt_socks[NUM_OF_GATE];
int share = 0;
time_t t = time(NULL);


// 소켓 함수 오류 출력 후 종료
void err_quit(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

// 소켓 함수 오류 출력
void err_display(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

// 클라이언트와 데이터 통신
LPVOID ProcessClient(LPVOID arg)
{
    SOCKET client_sock = (SOCKET)arg;
    int retval;
    int retval2;
    SOCKADDR_IN clientaddr;
    int addrlen;
    char buf[BUFSIZE + 1];
    char buf2[BUFSIZE + 1];
    int i = 0;



    // 클라이언트 정보 얻기
    addrlen = sizeof(clientaddr);
    getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);

    while (1) {
        // 데이터 받기
        retval = recv(client_sock, buf, BUFSIZE, 0);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }
        else if (retval == 0)
            break;



        // 받은 데이터 출력
        buf[retval] = '\0';
        printf("[TCP/%s:%d] 받은값 %s \n", inet_ntoa(clientaddr.sin_addr),
            ntohs(clientaddr.sin_port), buf);




        // 데이터 보내기
        retval = send(client_sock, buf, retval, 0);
        if (retval == SOCKET_ERROR) {
            err_display("send()");
            break;
        }

        printf("[TCP/%s:%d] 보낸값 %s\n", inet_ntoa(clientaddr.sin_addr),
            ntohs(clientaddr.sin_port), buf);
    }

    // closesocket()
    closesocket(client_sock);
    t = time(NULL);
    printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d 접속 종료 시간 : %s \n",
        inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port),ctime(&t));

    return 0;
}

int main(int argc, char* argv[])
{
    int retval;
    
    pthread_mutex_init(&mutx, NULL);

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // socket()
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) err_quit("listen()");

    // 데이터 통신에 사용할 변수
    SOCKET client_sock;
    SOCKADDR_IN clientaddr;
    int addrlen;
    HANDLE hThread;
    pthread_t t_id;

    while (1) {
        // accept()
        addrlen = sizeof(clientaddr);


        client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            err_display("accept()");
            break;
        }

        pthread_mutex_lock(&mutx);
        clnt_socks[clnt_cnt++] = client_sock;
        pthread_mutex_unlock(&mutx);

        pthread_create(&t_id, NULL, ProcessClient, (LPVOID)client_sock);
        pthread_detach(t_id);

        // 접속한 클라이언트 정보 출력
        printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d 현재 시간 : %s\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), ctime(&t));

    }

    // closesocket()
    closesocket(listen_sock);

    // 윈속 종료
    WSACleanup();
    return 0;
}
