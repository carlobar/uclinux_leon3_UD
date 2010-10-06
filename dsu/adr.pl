
print("wmem 0x90400024 0xf\n");
for ($i = 0;$i < 8 * 32;$i++) {
    printf("wmem 0x907%.5x 0x%.4x%.4x\n",$i*4,$i/8,$i%8);
} 
print("flush\n");
