#include <iostream>
#include <stdexcept>
#include <occi.h>

using namespace std;
using namespace oracle::occi;

int main() {
    Environment* env = nullptr;
    Connection* conn = nullptr;

    try {
        cout << "Criando ambiente OCCI..." << endl;
        env = Environment::createEnvironment(Environment::DEFAULT);

        cout << "Tentando conectar ao Oracle..." << endl;
        conn = env->createConnection("user", "senha", "localhost:1521/xe");

        cout << "Conexao estabelecida com sucesso!" << endl;

        cout << "Criando comando SQL..." << endl;
        Statement* stmt = conn->createStatement("SELECT id_func, nome, salario, data_admissao FROM funcionarios");

        cout << "Executando consulta..." << endl;
        ResultSet* rs = stmt->executeQuery();

        cout << "Lendo resultados da tabela..." << endl;
        bool encontrou = false;

        while (rs->next()) {
            encontrou = true;

            int id = rs->getInt(1);
            string nome = rs->getString(2);
            double salario = rs->getDouble(3);
            Date admissao = rs->getDate(4);

            cout << "ID: " << id
                << " | Nome: " << nome
                << " | Salario: " << salario
                << " | Admissao: " << admissao.toText("DD/MM/YYYY")
                << endl;

        }

        if (!encontrou) {
            cout << "A consulta retornou 0 linhas." << endl;
        }

        cout << "Fechando recursos..." << endl;
        stmt->closeResultSet(rs);
        conn->terminateStatement(stmt);
        env->terminateConnection(conn);
        Environment::terminateEnvironment(env);
    }
    catch (SQLException& e) {
        cout << "ERRO ORACLE (OCI/OCCI): " << e.getMessage() << endl;
    }
    catch (exception& e) {
        cout << "ERRO C++: " << e.what() << endl;
    }
}
