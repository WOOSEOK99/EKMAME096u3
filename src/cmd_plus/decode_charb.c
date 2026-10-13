			while (*c)
			{
				if (*c == '_')
				{
					*c++ = 0x90;
					switch (*c)
					{
						case 'A':
						case 'B':
						case 'C':
						case 'D':
							*c = *c - 'A' + 0x01 + c1;
							break;
						case 'E':
						case 'F':
						case 'G':
						case 'H':
						case 'I':
						case 'J':
							*c = *c - 'E' + 0x1d + c1;
							break;
						case 'K':
							*c = 0x06 + c1;
							break;
						case 'L':
						case 'M':
						case 'O':
						case 'Q':
						case 'R':
							*c = *c - 'L' + 0x4a + c1;
							break;
						case 'N':
							*c = 0x18 + c1;
							break;
						case 'P':
							*c = 0x05 + c1;
							break;
						case 'S':
							*c = 0x07 + c1;
							break;
						case 'T':
						case 'U':
						case 'V':
						case 'W':
						case 'X':
						case 'Y':
						case 'Z':
							*c = *c - 'T' + 0x23 + c1;
							break;
						case 'a':
						case 'b':
						case 'c':
						case 'd':
						case 'e':
						case 'f':
						case 'g':
						case 'h':
						case 'i':
						case 'j':
							*c = *c - 'a' + 0x09 + c1;
							break;
						case 'k':
						case 'l':
						case 'm':
						case 'n':
						case 'o':
						case 'p':
						case 'q':
						case 'r':
						case 's':
						case 't':
						case 'u':
						case 'v':
						case 'w':
						case 'x':
						case 'y':
						case 'z':
							*c = *c - 'k' + 0x3a + c1;
							break;
						case '0':
						case '1':
						case '2':
						case '3':
						case '4':
						case '6':
						case '7':
						case '8':
						case '9':
							*c = *c - '0' + 0x13 + c1;
							break;
						case '5':
							*c = 0x2a + c1;
							break;
						case '`':
							*c = 0x2b + c1;
							break;
						case '@':
							*c = 0x2c + c1;
							break;
						case '(':
							*c = 0x2d + c1;
							break;
						case ')':
							*c = 0x2e + c1;
							break;
						case '!':
							*c = 0x2f + c1;
							break;
						case '*':
							*c = 0x30 + c1;
							break;
						case '&':
							*c = 0x31 + c1;
							break;
						case '^':
							*c = 0x32 + c1;
							break;
						case '%':
							*c = 0x33 + c1;
							break;
						case '$':
							*c = 0x34 + c1;
							break;
						case '#':
							*c = 0x35 + c1;
							break;
						case '<':
							*c = 0x36 + c1;
							break;
						case '>':
							*c = 0x37 + c1;
							break;
						case '{':
							*c = 0x38 + c1;
							break;
						case '}':
							*c = 0x39 + c1;
							break;
						case '+':
							*c = 0x08 + c1;
							break;
						case '|':
							*c = 0x4c + c1;
							break;
						case '?':
							*c = 0x4e + c1;
							break;
						case '-':
							*c = 0x51 + c1;
							break;
						case '=':
							*c = 0x52 + c1;
							break;
						case ']':
							*c = 0x53 + c1;
							break;
						case '[':
							*c = 0x54 + c1;
							break;
						case '~':
							*c = 0x55 + c1;
							break;
					}
				}

				c++;
			}
