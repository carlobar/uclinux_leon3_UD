#!/bin/bash
# Copyright (C) 2010 Carlos Barreto

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.



FILE=erase_ram 		# file name
WORD_SIZE=32 		# bits
RAM_SIZE=67108864 	# bytes
COMMAND="wmem " 	# write in memory
DATA="0x00000000" 	# 32-bit word
START=0x40000000
N_COMMANDS=$(($RAM_SIZE/($WORD_SIZE/8)))
ADDR=$START
#ADDR_A = ('0', '4', '8', 'c')

rm $FILE

echo -e "\n Generator of code to erase the\n RAM memory with GRMON" 
echo -e " Word size = $WORD_SIZE \n RAM size = $RAM_SIZE \n Number of commands to print= $N_COMMANDS"

for ((i = 0; i <= $N_COMMANDS; i++ )); do

	T=$(printf "%s %#x %i" "$COMMAND" "$ADDR" "$DATA")
	echo -n -e "$T\n" >> $FILE
	ADDR=$(($ADDR + 4))
done

echo "flush" >> $FILE
exit 0
