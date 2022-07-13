//-클라이언트가 서버에 접속하면 서버측 화면에
//
//.클라이언트의 IP주소와 포트번호를 출력
//
//.현재의 날짜와 시간을 “XXXX년도 XX월 XX시 XX분 XX초" 형식으로 보여줌
//
//- 클라이언트가 문자열을 입력한 후 엔터키를 치면, 서버로 전송하고
//
//- 서버는 클라이언트로부터 받은 문자열를 서버측 화면에 보여주고, 그 문자열을 다시 클라이언트에 전송
//
//- 클라이언트는 서버로부터 받은 문자열을 클라이언트는 화면에 출력
//
//- 클라이언트가 연결해제를 하면, 서버 측에서는 해당 클라이언트의 P주소와 포트번호, 연결해제 요청 시간을 보여줌
//
//- 모든 문자를 입력받을 수 있다. (특수문자 제외가능)
//
//- 클라이언트는 3개이상 접속
//
//- 연결요청, 연결해제, 실제 데이터 송수신의 세 가지 형태 사용

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
