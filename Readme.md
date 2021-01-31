# Compilador da linguagem Dartmouth Basic
Projeto para a disciplina PCS3866 - Linguagens e Compiladores
Felipe Fernandes de Lima

# Códigos tratados:
    Entrada: Dartmouth Basic
    Saída: Assembly para ARM7TDMI

# Módulos:
    Léxico: Faz o tratamento do código fonte de entrada para gerar lexemas previstos na linguagem
    Sintático: Interpreta construções sintáticas entre lexemas e produz a representação sintática do programa
    Semântico: Percorre os elementos sintáticos para buscar problemas de significado e processar expressões aritméticas
    Geração: Obtém a representação sintática tratada pelo semântico para convertê-la em código objeto
    Main: Módulo de chamada do programa. Também oferece funções para teste dos demais módulos

# Instruções de chamada:
    Chamada comum ao compilador:
        basicc <arquivo fonte> <arquivo objeto>

    Teste do Classificador ASCII:
        basicc <arquivo fonte> -A

    Teste do Analisador Léxico:
        basicc <arquivo fonte> -L

