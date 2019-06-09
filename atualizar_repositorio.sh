#!/bin/bash
echo "Atualizando reposítorio com as alterações do fork"
git pull
echo "Criando link com o repositório original"
git remote add upstream https://github.com/harleybarreto/iot-b26.git
git fetch upstream
echo "Atualizando repositório com o git original"
git rebase upstream/master
git push origin master	
