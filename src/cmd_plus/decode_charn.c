/**********************************************************************/
/* ui_font.c  decode character                                        */
/*  Normal Code                                                       */
/**********************************************************************/	
		if (*c == '_')
			{
				switch(*(c+1))
				{
					case 'A':
					case 'B':
					case 'C':
					case 'D':
						*c     = c1;
						*(c+1) = 0xc0 + (*(c+1) - 'A');
						c+=2;
						break;

					case 'P':
						*c     = c1;
						*(c+1) = 0xc4;
						c+=2;
						break;

					case 'K':
						*c     = c1;
						*(c+1) = 0xc5;
						c+=2;
						break;

					case 'S':
						*c     = c1;
						*(c+1) = 0xc6;
						c+=2;
						break;

					case '+':
						*c     = c1;
						*(c+1) = 0xc7;
						c+=2;
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
						*c     = c1;
						*(c+1) = 0xc8 + (*(c+1) - 'a');
						c+=2;
						break;

					case '1':
					case '2':
					case '3':
					case '4':
					case '6':
					case '7':
					case '8':
					case '9':
						*c     = c1;
						*(c+1) = 0xd2 + (*(c+1) - '0');
						c+=2;
						break;

					case 'N':
						*c     = c1;
						*(c+1) = 0xd7;
						c+=2;
						break;

					default:
						c++;
						break;
				}
			}
