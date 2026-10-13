#ifdef COMMAND_CONVERT_TEXT
			if (*c == COMMAND_CONVERT_TEXT)
			{
				i = 0;
				while (convert_text[i].org_name)
				{
					if (*(c+1) == convert_text[i].org_name[1] &&
						*(c+2) == convert_text[i].org_name[2] &&
						*(c+3) == convert_text[i].org_name[3] )
					{
						*c     = convert_text[i].fix_name[0];
						*(c+1) = convert_text[i].fix_name[1];
						*(c+2) = NULL_SPACE1;
						*(c+3) = NULL_SPACE1;
						break;
					}
					i++;
				}
			}
#endif /* COMMAND_CONVERT_TEXT */

#ifdef COMMAND_DEFAULT_TEXT
			if (*c == COMMAND_DEFAULT_TEXT)
			{
				i = 0;
				while (default_text[i].org_name)
				{
					if (*(c+1) == default_text[i].org_name[1])
					{
						*c     = c1 + (default_text[i].fix_name / s1);
						*(c+1) = c2 + (default_text[i].fix_name % s1);
						c++;
						break;
					}
					i++;
				}
				c++;
			}
#endif /* COMMAND_DEFAULT_TEXT */

#ifdef COMMAND_EXPAND_TEXT
			else if (*c == COMMAND_EXPAND_TEXT)
			{
				i = 0;
				while (expand_text[i].org_name)
				{
					if (*(c+1) == expand_text[i].org_name[1])
					{
						*c     = c1 + (expand_text[i].fix_name / s1);
						*(c+1) = c2 + (expand_text[i].fix_name % s1);
						c++;
						break;
					}
					i++;
				}
				c++;
			}
#endif /* COMMAND_EXPAND_TEXT */

#ifdef COMMAND_LOGO_MARK
			else if (*c == COMMAND_LOGO_MARK)
			{
				i = 0;
				while (logo[i].org_name)
				{
					if (*(c+1) == logo[i].org_name[1] &&
						*(c+2) == logo[i].org_name[2] &&
						*(c+3) == logo[i].org_name[3] &&
						*(c+4) == logo[i].org_name[4] &&
						*(c+5) == logo[i].org_name[5] &&
						*(c+6) == logo[i].org_name[6] &&
						*(c+7) == logo[i].org_name[7] )
						{
							*c     = c1 + (logo[i].fix_name / s1);
							*(c+1) = c2 + (logo[i].fix_name % s1);
							*(c+2) = c1 + ((logo[i].fix_name + 1) / s1);
							*(c+3) = c2 + ((logo[i].fix_name + 1) % s1);
							*(c+4) = c1 + ((logo[i].fix_name + 2) / s1);
							*(c+5) = c2 + ((logo[i].fix_name + 2) % s1);
							*(c+6) = c1 + ((logo[i].fix_name + 3) / s1);
							*(c+7) = c2 + ((logo[i].fix_name + 3) % s1);
							c+=7;
							break;
						}
					i++;
				}

				i = 0;
				while (deco[i].org_name)
				{
					if (*(c+1) == deco[i].org_name[1] &&
						*(c+2) == deco[i].org_name[2] &&
						*(c+3) == deco[i].org_name[3] &&
						*(c+4) == deco[i].org_name[4] &&
						*(c+5) == deco[i].org_name[5] )
						{
							*c     = c1 + (deco[i].fix_name / s1);
							*(c+1) = c2 + (deco[i].fix_name % s1);
							*(c+2) = c1 + ((deco[i].fix_name + 1) / s1);
							*(c+3) = c2 + ((deco[i].fix_name + 1) % s1);
							*(c+4) = c1 + ((deco[i].fix_name + 2) / s1);
							*(c+5) = c2 + ((deco[i].fix_name + 2) % s1);
							c+=5;
							break;
						}
					i++;
				}
				c++;
			}
#endif /* COMMAND_LOGO_MARK */
