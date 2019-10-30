import sys

if len(sys.argv) not in [3,4]:
    print('Script for converting YAML generated by the reference software to fastq file(s)')
    print('Usage')
    print('SE')
    print('python yaml2fastq.py infile.yaml outfile.fastq')
    print('PE')
    print('python yaml2fastq.py infile.yaml outfile_1.fastq outfile_2.fastq')
    sys.exit()
elif len(sys.argv) == 3:
    fout1 = open(sys.argv[2],'w')
    PE = False
else:
    fout1 = open(sys.argv[2],'w')
    fout2 = open(sys.argv[3],'w')
    PE = True

record_segments_mode = False
with open(sys.argv[1]) as fin:
    for line in fin:
        line = line.rstrip('\n')
        if record_segments_mode:
            if "quality_values:" in line:
                line = fin.readline()
                line = fin.readline()
                quality_list.append(line[5:-2])
            if "sequence" in line:
                read_list.append(line[14:-1])
            if "alignments" in line:
                record_segments_mode = False
                if PE:
                    if number_of_record_segments == 1 and read_1_first:
                        fout1.write(read_name+'/1\n'+read_list[0]+'\n+\n'+quality_list[0]+'\n')
                    elif number_of_record_segments == 1 and not read_1_first:
                        fout2.write(read_name+'/2\n'+read_list[0]+'\n+\n'+quality_list[0]+'\n')
                    elif number_of_record_segments == 2 and read_1_first:
                        fout1.write(read_name+'/1\n'+read_list[0]+'\n+\n'+quality_list[0]+'\n')
                        fout2.write(read_name+'/2\n'+read_list[1]+'\n+\n'+quality_list[1]+'\n')
                    else:
                        fout1.write(read_name+'/1\n'+read_list[1]+'\n+\n'+quality_list[1]+'\n')
                        fout2.write(read_name+'/2\n'+read_list[0]+'\n+\n'+quality_list[0]+'\n')
                else:
                    fout1.write(read_name+'\n'+read_list[0]+'\n+\n'+quality_list[0]+'\n')

        if "number_of_record_segments" in line:
            number_of_record_segments = int(line[29])
        elif "read_1_first" in line:
            read_1_first = int(line[16])
        elif "read_name" in line:
            read_name = line[14:-1]
        elif "record_segments" in line:
            record_segments_mode = True
            quality_list = []
            read_list = []
