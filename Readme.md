# Extensões ao Compilador da linguagem Dartmouth Basic
Projeto para a disciplina PCS3866 - Linguagens e Compiladores
Felipe Fernandes de Lima

# Extensões adicionadas
    Expressões aritméticas:
        - Tratamento do sinal de menos unário
        - Operação de potenciação com precedência sobre as demais
        - Funções com zero ou mais parâmetros
    Analisador Léxico como reconhecedor descendente
    Analisador Sintático com recuperação de erros do tipo Modo de Pânico

# Códigos tratados:
    Entrada: Dartmouth Basic
    Saída: Assembly para ARM7TDMI

# Instruções de chamada:
    Chamada comum ao compilador:
        basicc <arquivo fonte> <arquivo objeto>

    Teste do Classificador ASCII:
        basicc <arquivo fonte> -A

    Teste do Analisador Léxico:
        basicc <arquivo fonte> -L

    Teste do Analisador Sintático:
        basicc <arquivo fonte> -S

