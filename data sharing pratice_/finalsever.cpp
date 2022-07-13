//����
//��� �ݹ� �����ߴµ� �߰��� ����(?)�� �Ǽ��� �׳� �� ���ƾ�����ȴ� 
//������ �Ǽ��� ���� �����ϰ� ����� �Ǽ��ε� �Լ��� �߸����� �� �ȵǳİ� �� �ڽ����� ������ �־���.
// 
// ���ؽ��� �̿��ϰ��� �ߴµ� ���� �ٽ� �Լ������ϱ� ������ lock�� unlock�� ���� ����ϱ����� pthread�� ����ߴ�
// �׷��� ���� pthread�� ���� ��ġ���� �ʴ´ٸ� �ڵ� ������ �Ұ����ϴ� �����ߴٸ� ���ڴ�.
// �ڵ� ������ �ַ� pthread�� ���ؽ��� ����Ͽ� lock�� unlock�� ����ߴ�.  Ŭ���̾�Ʈ�� ������ �����Ҷ� ������ �ϳ��� ����� �ű⿡ �ּҸ� �ְ� �״�� �����带 ����� �������� �����ߴ�.
// 
//�̷������� �����ߴµ� ���������� �ƴ� �̰� ���� ��������� �� �𸣰ڴ�. �ٸ� ������ share���� ���������� �ٲ�� Ŭ���̾�Ʈ�� �ѹ��� ��ĥ������ share+10���� �ް� share+10���� ������ Ŭ���̾�Ʈ����
// �� �ް��ִ��� Ȯ���� �����ߴ�
//pthread�� ���� ������� �ʰ� ó���� ���� �ڵ忡�ٰ� ���� ó���� �����ߴ� �ڵ��� �߰��� ����ص� �� ������ �ɰ� ���� ������ ������ �ð��� ���� �׽�Ʈ�� ���߱⿡ Ȯ������ �ʴ�.
// 
// 
//�׸��� ��� �ְ�޴� ���� �̷������� ó���ص� ���������� �� �𸣰ڴ�. ������ 100�ۼ�Ʈ �������� ����        �� ���� ũ��
// ������� sever���ִ� share ���� ó���� 0���� �ʱ�ȭ�� ���ְ� �̰��� client���� �ּҰ��� ������ share�� client���� �����ְ� share���� ���� client�� sever���� ���� share�� +10�� �ؼ� �����ָ�
//sever�� �ִ� share���� 20���� ���Ѵٴ°��� ���� ������ ���̴�.

#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
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


// ���� �Լ� ���� ��� �� ����
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

// ���� �Լ� ���� ���
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

// Ŭ���̾�Ʈ�� ������ ���
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



    // Ŭ���̾�Ʈ ���� ���
    addrlen = sizeof(clientaddr);
    getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);

    while (1) {
        // ������ �ޱ�
        retval = recv(client_sock, buf, BUFSIZE, 0);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }
        else if (retval == 0)
            break;



        // ���� ������ ���
        buf[retval] = '\0';
        printf("[TCP/%s:%d] ������ %s \n", inet_ntoa(clientaddr.sin_addr),
            ntohs(clientaddr.sin_port), buf);




        // ������ ������
        retval = send(client_sock, buf, retval, 0);
        if (retval == SOCKET_ERROR) {
            err_display("send()");
            break;
        }

        printf("[TCP/%s:%d] ������ %s\n", inet_ntoa(clientaddr.sin_addr),
            ntohs(clientaddr.sin_port), buf);
    }

    // closesocket()
    closesocket(client_sock);
    t = time(NULL);
    printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d ���� ���� �ð� : %s \n",
        inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port),ctime(&t));

    return 0;
}

int main(int argc, char* argv[])
{
    int retval;
    
    pthread_mutex_init(&mutx, NULL);

    // ���� �ʱ�ȭ
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

    // ������ ��ſ� ����� ����
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

        // ������ Ŭ���̾�Ʈ ���� ���
        printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d ���� �ð� : %s\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), ctime(&t));

    }

    // closesocket()
    closesocket(listen_sock);

    // ���� ����
    WSACleanup();
    return 0;
}
