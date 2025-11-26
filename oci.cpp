#include <iostream>
#include <oci.h>

using namespace std;

//  ponteiros opcionais
OCIEnv* g_env = nullptr;
OCIError* g_err = nullptr;
OCISvcCtx* g_svc = nullptr;

void checkerr(OCIError* errhp, sword status) {
	if (status != OCI_SUCCESS) {
		text errbuf[512];
		sb4 errcode = 0;

		OCIErrorGet(errhp, 1, nullptr, &errcode, errbuf,
			sizeof(errbuf), OCI_HTYPE_ERROR);


		cout << "Erro OCI ("<< errcode <<"): " << errbuf <<endl;
	}
}

void CconectarOracle()
{
    //1. Iniciar OCI
    checkerr(nullptr, OCIEnvCreate(&g_env, OCI_DEFAULT, 0, 0, 0, 0, 0, 0));

    //2. Aloca um handle de erro
    OCIHandleAlloc(g_env, (void**)&g_err, OCI_HTYPE_ERROR, 0, 0);

    //3. Criar o contexto (conexão)
    OCIHandleAlloc(g_env, (void**)&g_svc, OCI_HTYPE_SVCCTX, 0, 0);

    //4. Login
    checkerr(
        g_err,
        OCILogon(
            g_env, g_err, &g_svc,
            (OraText*)"user", 6, //usuario
            (OraText*)"senha", 8, //senha
            (OraText*)"localhost/XE", 12 //string de conexão
        )
    );
    cout << "Conetado ao Oracle com sucesso!" << endl;
}
void desconectarOracle() {
    if (g_svc != nullptr && g_err != nullptr) {
        OCILogoff(g_svc, g_err);
        cout << "Desconectado do Oracle com sucesso!" << endl;
    }
}


void inserirAluno(const string& nome, const string& email) {
    OCIStmt* stmt = nullptr;
    OCIBind* bndNome = nullptr;
    OCIBind* bndEmail = nullptr;

    const char* sql = "INSERT INTO TB_ALUNOS (NOME, EMAIL) VALUES (:nome, :email)";

    // Alocar stmt
    checkerr(g_err,
        OCIHandleAlloc(g_env, (void**)&stmt, OCI_HTYPE_STMT, 0, nullptr)
    );

    // Preparar SQL
    checkerr(g_err,
        OCIStmtPrepare(stmt, g_err,
            (OraText*)sql, (ub4)strlen(sql),
            OCI_NTV_SYNTAX, OCI_DEFAULT)
    );

    // Buffers
    char nomeBuf[101] = {0};
    strncpy(nomeBuf, nome.c_str(), sizeof(nomeBuf) - 1);

    char emailBuf[101] = {0};
    strncpy(emailBuf, email.c_str(), sizeof(emailBuf) - 1);

    // Bind Nome
    checkerr(g_err,
        OCIBindByName(stmt, &bndNome, g_err,
            (OraText*)":nome", -1,
            (dvoid*)nomeBuf, sizeof(nomeBuf),
            SQLT_STR, nullptr, nullptr, nullptr, 0, nullptr, OCI_DEFAULT)
    );

    // Bind Email
    checkerr(g_err,
        OCIBindByName(stmt, &bndEmail, g_err,
            (OraText*)":email", -1,
            (dvoid*)emailBuf, sizeof(emailBuf),
            SQLT_STR, nullptr, nullptr, nullptr, 0, nullptr, OCI_DEFAULT)
    );

    // Executar com commit
    checkerr(g_err,
        OCIStmtExecute(g_svc, stmt, g_err, 1, 0, nullptr, nullptr, OCI_COMMIT_ON_SUCCESS)
    );

    cout << "Aluno inserido com sucesso: " << nome << ", email: " << email << endl;

    OCIHandleFree(stmt, OCI_HTYPE_STMT);
}



void listarAlunos() {
    OCIStmt* stmt = nullptr;
    OCIDefine* defId = nullptr;
    OCIDefine* defNome = nullptr;
    OCIDefine* defEmail = nullptr;

    const char* sql = "SELECT ID, NOME, EMAIL FROM TB_ALUNOS";

    // Alocar stmt
    checkerr(g_err,
        OCIHandleAlloc(g_env, (void**)&stmt, OCI_HTYPE_STMT, 0, nullptr)
    );

    // Preparar SQL
    checkerr(g_err,
        OCIStmtPrepare(stmt, g_err,
            (OraText*)sql, (ub4)strlen(sql),
            OCI_NTV_SYNTAX, OCI_DEFAULT)
    );

    // Definir saída para ID
    sb4 idValor;
    checkerr(g_err,
        OCIDefineByPos(stmt, &defId, g_err,
            1, (dvoid*)&idValor, sizeof(idValor),
            SQLT_INT, nullptr, nullptr, nullptr, OCI_DEFAULT)
    );

    // Definir saída para Nome
    char nomeBuf[101] = { 0 };
    checkerr(g_err,
        OCIDefineByPos(stmt, &defNome, g_err,
            2, (dvoid*)nomeBuf, sizeof(nomeBuf),
            SQLT_STR, nullptr, nullptr, nullptr, OCI_DEFAULT)
    );

    // Definir saída para Email
    char emailBuf[101] = { 0 };
    checkerr(g_err,
        OCIDefineByPos(stmt, &defEmail, g_err,
            3, (dvoid*)emailBuf, sizeof(emailBuf),
            SQLT_STR, nullptr, nullptr, nullptr, OCI_DEFAULT)
    );

    // Executar o SQL
    checkerr(g_err,
        OCIStmtExecute(g_svc, stmt, g_err, 0, 0, nullptr, nullptr, OCI_DEFAULT)
    );

    // Loop de leitura
    cout << "Lista de Alunos:" << endl;
    while (true) {
        sword status = OCIStmtFetch2(stmt, g_err, 1, OCI_DEFAULT, 0, OCI_DEFAULT);
        if (status == OCI_NO_DATA) break;
        if (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO) {
            checkerr(g_err, status);
            break;
        }

        cout << "ID: " << idValor << ", Nome: " << nomeBuf << ", Email: " << emailBuf << endl;
    }

    // Liberar stmt
    OCIHandleFree(stmt, OCI_HTYPE_STMT);
}


void menu() {
    int op = -1;

    do {
        cout << "Menu de OP:" << endl;
        cout << "1. Inserir Aluno" << endl;
        cout << "2. Listar Alunos" << endl;
        cout << "0. Sair" << endl;
        cout << "Escolha uma opcao: ";
        cin >> op;

        switch (op) {
        case 1: {
            string nome;
            string email;
            cout << "Digite o nome do aluno: ";
            cin >> nome;
            cout << "Digite a email do aluno: ";
            cin >> email;
            inserirAluno(nome, email);
            break;
        }
        case 2: {
            listarAlunos();
            break;
        }
        case 0: {
            cout << "Saindo..." << endl;
            break;
        }
        default: {
            cout << "OP invalida!" << endl;
        }
        }
    } while (op != 0);
}

int main() {
    CconectarOracle();
    menu();
    desconectarOracle();
    return 0;
}


/*
int main() {
    // Inicia OCI
    checkerr(nullptr, OCIEnvCreate(&env, OCI_DEFAULT, 0, 0, 0, 0, 0, 0));

    // Alocar manipulador para erro
    OCIHandleAlloc(env, (void**)&err, OCI_HTYPE_ERROR, 0, 0);

    // Criar o contexto (conexao com banco)
    OCIHandleAlloc(env, (void**)&g_svc, OCI_HTYPE_SVCCTX, 0, nullptr);
    // Login
    checkerr(
        err,
        OCILogon(
            env, err, &g_svc,
            (OraText*)"user", 6, //user
            (OraText*)"senha", 8, //senha
            (OraText*)"localhost/XE", 12 //string conexao
        )
    );

    cout << "Conectado ao Oracle com sucesso!" << endl;

    // Preparar o SQL
    OCIStmt* stmt = nullptr;
    OCIHandleAlloc(env, (void**)&stmt, OCI_HTYPE_STMT, 0, 0);

    const char* sql = "SELECT * FROM funcionarios";

    checkerr(err, OCIStmtPrepare(stmt, err,
        (OraText*)sql, (ub4)strlen(sql),
        OCI_NTV_SYNTAX, OCI_DEFAULT));

    // Definir a saida
    char resultado[50];
    OCIDefine* def = nullptr;
    checkerr(err,
        OCIDefineByPos(stmt, &def, err,
            1, resultado, sizeof(resultado),
            SQLT_STR, nullptr, nullptr, nullptr, OCI_DEFAULT));

    // Executar o SQL
    checkerr(err, OCIStmtExecute(
        g_svc, stmt, err, 0, 0, nullptr, nullptr, OCI_DEFAULT));

    // Loop de leitura
    while (OCIStmtFetch2(stmt, err, 1, OCI_DEFAULT, 0, OCI_DEFAULT) != OCI_NO_DATA) {
        cout << "Resultado : " << resultado << endl;
    }

    // Finaliza a sessao
    OCILogoff(g_svc, err);
    cout << "Sessao encerrada" << endl;
}
*/
