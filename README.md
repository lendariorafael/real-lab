# Real Lab

Plugin VST3 para Windows (FL Studio 26): sampler e sintetizador com presets por género e instrumento, imagem em cada preset e efeitos ajustados ao estilo de cada género.

## Géneros e presets

- **Afrobeat, Pop, Trap, Drill, Zouk**: separadores no topo.
- **Meus**: os teus presets gravados (ficheiros XML em `%APPDATA%\RealLab\Presets`).

Há 40 presets de fábrica: 10 de piano, 10 de guitarra e violão, 10 de flauta e 10 de baixo, com 2 por género em cada instrumento.

## Efeitos

Filter, Vibrato, Gate, Character, Reverb e Delay. Cada efeito tem um ícone próprio na secção. Cada preset já traz os seus valores para o género e instrumento.

## Como acrescentar uma imagem

1. Coloca o ficheiro `.jpg` (de preferência 16:9, 1600×900) na pasta `Assets/` com o nome indicado no preset (por exemplo `flauta_01.jpg`).
2. Não precisas de editar mais nada: o CMake inclui automaticamente todos os `.jpg` de `Assets/`.
3. Enquanto a imagem não existir, o preset mostra um cartão com o nome do instrumento.

## Compilação

O GitHub Actions compila automaticamente em cada push (ver separador **Actions**). O artefacto **RealLab-VST3-Windows** contém `Real Lab.vst3`.

Instalação: copia a pasta `Real Lab.vst3` para `C:\Program Files\Common Files\VST3\` e volta a procurar plugins no FL Studio.

## Licença

Usa JUCE 8.0.4 (GPLv3 ou comercial). Para uso pessoal, a versão gratuita serve.
