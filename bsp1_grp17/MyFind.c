/**
 * @myfind.c
 * Betriebssysteme myfind
 * Beispiel 1
 *
 * @author Patrick Hamilton
 * @author René Kucera
 * @author Maximillian Eggerstorfer
 *
 * @version $Revision:1
 . @date 3/7/2014
 *
 */
/*
 * -- includes --
 */
#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <malloc.h>
#include <stdbool.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <grp.h>
#include <time.h>
#include <errno.h>
#include <fnmatch.h>
#include <unistd.h>
#include <libgen.h>
/*
 * -- defines --
 */


/* FB Gruppe7: Limitierung des Pfades nicht nötig, Platz für das Array könnte mit malloc etc. bereitgestellt werden */

/**
 * \def MAX_PATH
 * \brief Pfadlaenge wird beschraenkt
 *
 * Der zu durchsuchende Pfad wird hier beschraenkt. Es koennen maximal 2047 Zeichen
 * Pfadlaenge verwendet werden (steuerungszeichen '\0' ist das letzte Zeichen)
 *
 */
#define MAX_PATH 8096

/* FB Gruppe7: Symbolische Konstanten sind hier sehr sinnvoll */

#define PARAM_USER "-user"
#define PARAM_LS "-ls"
#define PARAM_TYPE "-type"
#define PARAM_PRINT "-print"
#define PARAM_NOUSER "-nouser"
#define PARAM_NAME "-name"
#define PARAM_PATH "-path"
/*
 * -- typedefs --
 */
/*
 * -- globals --
 */

typedef struct myFindValues
{
	int ParameterCount;
	char Programmname[MAX_PATH];
	char Path[MAX_PATH];
} myFindValues;


/**
 * \var params
 * \brief Enthaelt globale Parameter, die im gesamten Programm benoetigt werden
 *
 * \param ParameterCount enthaelt die Anzahl der uebergebenen Parameter
 * \param Programmname enthaelt den Programmnamen
 * \param Path enthaelt den uebergebenen Suchpfad
 */
myFindValues params;

/*
 * -- functions --
 */
int printDirectory(const char *dir, int level, int fpOutputfile, char *searchOptions[]);
bool checkSearchOptions(char *searchOptions[]);
void checkPath(char *options[], char *result);
void findWithArguments(struct dirent *dirp, char *argv[], char *path,
		struct stat inputfileStat);
void print(char *path);
struct passwd getuser(char *username);
struct passwd getuserpername(char *username);
struct passwd getuserperid(char *username);
struct group getgroup(char *groupname);
void getprotection(struct stat inputfileStat);
void gettime(struct stat inputfileStat);
bool checkIfUserExists(char *username);
bool typeValidation (char *searchOptions);

/**
 *
 *\brief Main-Funktion des abgespeckten "Find".
 *
 *Alle Uebergabeparameter (falls vorhanden) werden in der Funktion checkSearchOptions geprueft
 *\param argc Anzahl der uebergabeparameter
 *\param argv Uebergabeparameter (inkludiert den namen des Programms argv[0]
 *
 *\return Wenn das Programm die Suche durchführt dann "Success", sonst "Failure"
 *\retval EXIT_SUCCESS
 *\retval EXIT_FAILURE
 */
int main(int argc, char* argv[])
{
	int fpOutputfile = 0;

	/* FB Gruppe7: strcpy() ohne Überprüfung auf Länge */	
	strcpy(params.Programmname, argv[0]);
	params.ParameterCount = argc;

	if (checkSearchOptions(argv) == true)
	{
		/* strcpy()
		 * Laut Manual: No errors are defined */
		checkPath(argv, params.Path);
		
		printDirectory(params.Path, 0, fpOutputfile, argv);
		
		exit(EXIT_SUCCESS);
	}
	else
	{
		exit(EXIT_FAILURE);
	}
}
/**
 * \brief Die Funktion printDirectory geht ein File/Directory nach dem Anderen durch
 *
 * Geht rekursiv vor. Jeder Festplatteneintrag wird mit den Suchparameter geprueft
 *
 * \param *dir
 * \param level integer zum Merken des Strukturlevels (fuer Rekursion)
 * \param *searchOptions[] Suchparameter zum pruefen des Files/Directorys
 * \param fpOutputfile
 * \retval EXIT_SUCCESS
  *\retval EXIT_FAILURE
 */

/* FB Gruppe7: initialer Aufruf von main() aus erfolgt mit fpOutputfile=0, wird im Laufe der Ausführung von printDirectory() nie verändert und rekursive Aufrufe erfolgen auch mit diesem Wert, der immer "0" ist */
int printDirectory(const char *dir, int level, int fpOutputfile, char *searchOptions[])
{
	DIR *dpDirectory = NULL;
	struct dirent *dirp = NULL;
	struct stat inputfileStat;
	char completePath[MAX_PATH] = "\0";
	
	/* Wenn der uebergebene Pfad in 'const char *dir' ein Pfad+Filename ist dann mit dem stat das einzelne File testen.
	 * dirp wird für den Aufruf findWithArguments benoetigt, ist in dem Fall 'Einzelnes File' nicht verfuegbar. Darum wird es als 
	 * NULL uebergeben. 
	 */

	/* FB Gruppe7: ernno wird nicht zurückgesetzt */
	stat(dir, &inputfileStat);
			
	if (S_ISREG(inputfileStat.st_mode) != 0)
	{
		if(params.ParameterCount > 2)
		{
			strcpy(completePath, dir);
			findWithArguments(dirp, searchOptions, completePath, inputfileStat);
		}
		else
		{
			printf("%s\n",dir);
		}
		return EXIT_SUCCESS;
	}
	/* Ordner wurde im 'dir' uebergeben
	 * oeffnen des Verzeichnisses oder Fehlermeldung 
	 */
	if ((dpDirectory = opendir(dir)) == NULL)
	{
		fprintf(stderr, "%s: %s\n",params.Programmname, strerror(errno));
		return EXIT_FAILURE;
	}

	/* Verzeichnis durchgehen und ausgeben ob eine Datei oder ein Ordner gefunden wurde. */
	while ((dirp = readdir(dpDirectory)) != NULL)
	{
		/* Erzeugen des absoluten Pfades aus Hauptverzeichnis und Dateinamen */
		completePath[0] = '\0';
		strncat(completePath, dir, MAX_PATH - strlen(completePath)); 
		/* MAX_PATH - strlen(completePath) == maximale Anzahl an noch moeglichen chars im Array*/
		strncat(completePath, dirp->d_name, MAX_PATH - strlen(completePath));

		/* Erzeugen der Stat */
		if (stat(completePath, &inputfileStat) < 0)
		{
			fprintf(stderr, "%s: %s\n",params.Programmname, strerror(errno));
			exit(EXIT_FAILURE);
		}
		
		if (S_ISREG(inputfileStat.st_mode) != 0)
		{
			findWithArguments(dirp, searchOptions, completePath, inputfileStat);
		}
		else if (S_ISDIR(inputfileStat.st_mode) != 0)
		{
			/* Wenn ein Unterordner vorhanden ist der nicht . oder .. ist dann geh hinein */
			if (strcmp(dirp->d_name, "..") != 0 && strcmp(dirp->d_name, ".") != 0)
			{
				if (params.ParameterCount != 1)
				{
					findWithArguments(dirp, searchOptions, completePath, inputfileStat);
					strncat(completePath, "/", MAX_PATH - strlen(completePath));
					printDirectory(completePath, level + 1, fpOutputfile, searchOptions);
				}
				else
				{
					printf("%s\n", completePath);
				}

			}
		}
		else if (S_ISLNK(inputfileStat.st_mode) != 0)
		{
			findWithArguments(dirp, searchOptions, completePath, inputfileStat);
		}
	}

	/* Schliesen des Verzeicnisses */
	if (dpDirectory != NULL )
	{
		if (closedir(dpDirectory) < 0)
		{
			if (closedir(dpDirectory) < 0)
			{
				fprintf(stderr, "%s: Error: Close directory failed. %s", params.Programmname, strerror(errno));
				exit(EXIT_FAILURE);
			}
			else
				dpDirectory = NULL;
		}
		return EXIT_SUCCESS;
	}
	return EXIT_SUCCESS;
}
/**
 * \brief Die Funktion checkSearchOptions validiert die Suchparameter.
 *
 * Zuerst erfolgt Pruefung ob ein Parameter uebergeben wurde, anschließend werden alle uebergebenen Parameter mit den validen Suchparameter verglichen.
 * Wurde kein Parameter uebergeben wird der Rueckgabewert der checkSearchOptions Funktion auf false gesetzt. 
 *
 * \param **searchOptions
 * \retval true 
 * \retval false
 */
bool checkSearchOptions(char **searchOptions)
{
	bool result = true;
	int i = 2;
	int userFound = 0;
	
	if(params.ParameterCount < 2)
	{
		fprintf(stderr, "Aufrufvorsschrift: %s <DIRECTORY> [PARAMETER]\n", params.Programmname);
		return (EXIT_FAILURE);
	}

	while(i < params.ParameterCount)
	{
		
		if(searchOptions[i] == NULL)
		{
			fprintf(stderr,"%s: '%s'\n",params.Programmname, strerror(errno));
			return (EXIT_FAILURE);
		}
		
		if(strcmp(searchOptions[i], PARAM_LS) == 0 ||
		   strcmp(searchOptions[i], PARAM_PRINT) == 0 ||
                   strcmp(searchOptions[i],PARAM_NOUSER) == 0 )
		{
			i++;
			continue;
		}
		
		if (strcmp(searchOptions[i], PARAM_NAME) == 0)
		{
			if ((i + 1 < params.ParameterCount) == 0)
			{
				fprintf(stderr,"%s: Missing argument to '%s'\n",params.Programmname, searchOptions[i]);
				return (EXIT_FAILURE);
			}
			else
			{
				/* FB Gruppe7: i+=2 nachvollziehbar da nächster Eintrag Argument zum aktuellen Eintrag ist. Hinweis darauf wäre aber hilfreich */
				i += 2;
				continue;
			}
		}
		else if (strcmp(searchOptions[i], PARAM_USER) == 0)
		{
			if ((i + 1 < params.ParameterCount) == 0)
			{
				fprintf(stderr,"%s: Missing argument to '%s'\n",params.Programmname, searchOptions[i]);
				exit(EXIT_FAILURE);
			}
			else if (!checkIfUserExists(searchOptions[i + 1]))
			{
				fprintf(stderr,"%s: User '%s' does not exist\n", params.Programmname, searchOptions[i+1]);
				exit(EXIT_FAILURE);
			}
			else
			{
				/* FB Gruppe7: Für was ist diese Variable gut? Wird initialisiert und hier erhöht, aber sonst passiert nichts damit */
				userFound++;
				/* FB Gruppe7: i+=2 nachvollziehbar da nächster Eintrag Argument zum aktuellen Eintrag ist. Hinweis darauf wäre aber hilfreich */
				i += 2;
				continue;
			}
		}
		else if (strcmp(searchOptions[i], PARAM_TYPE) == 0)
		{
			if ((i + 1 < params.ParameterCount) == 0)
			{
				fprintf(stderr,"%s: Missing argument to '%s'\n", params.Programmname, searchOptions[i]);
				exit(EXIT_FAILURE);
			}
			if (typeValidation(searchOptions[i+1]) == false)
			{
				fprintf(stderr,"%s: Unknown argument to '%s'", params.Programmname, searchOptions[i]);
				return (EXIT_FAILURE);
			}
			else
			{
				/* FB Gruppe7: i+=2 nachvollziehbar da nächster Eintrag Argument zum aktuellen Eintrag ist. Hinweis darauf wäre aber hilfreich */
				i += 2;
				continue;
			}
		}
		else if (strcmp(searchOptions[i],PARAM_PATH) == 0)
		{
			if ((i + 1 < params.ParameterCount) == 0)
			{
				fprintf(stderr,"%s: Missing argument to '%s'\n",params.Programmname, searchOptions[i]);
				return (EXIT_FAILURE);
			}
			else
			{
				/* FB Gruppe7: i+=2 nachvollziehbar da nächster Eintrag Argument zum aktuellen Eintrag ist. Hinweis darauf wäre aber hilfreich */
				i += 2;
				continue;
			}
		}
		else 
		{
			fprintf(stderr,"%s: Unknown option '%s'\n", params.Programmname, searchOptions[i]);
			return (EXIT_FAILURE);
		}
	}
	/* FB Gruppe7: Nachdem result im Laufe der Ausführung nicht geändert wird, hätte man diese Variable nicht definieren brauchen und hier immer "return true" zurückgeben können */
	return result;
}
/**
 * \brief Die Funktion checkPath pruefft ob ein Pfad uebergeben wurde.
 * Wurde nichts uebergeben so wird als Pfad CurrentDirectory gesetzt.
 * Wurde der Pfad übergeben wird dieser als Suchdirectory gesetzt.
 *
 *
 * \param *options
 * \param *result
 */
void checkPath(char *options[], char *result)
{
	
		/* keine Parameter uebergeben -> setze SEARCH_PATH auf ./ */
	if (params.ParameterCount == 1)
	{
	/* FB Gruppe7: sollte "." sein */
		strcpy(result, "./");
	}
	/* FB Gruppe7: auch Pfade ohne "/" sind zulässig (-> jedes Subdirectory relativ zum PWD z.B.) */
	/* Check ob searchOptions[1] den SearchPath enthalten hat */
	else if (strchr(options[1], '/') != NULL)
	{
		strcpy(result, options[1]);
	}

	/* FB Gruppe7: sobald "-" ohne "/" im Pfad vorkommt, wird im PWD gesucht - Suche also in z.B. "meine-tollen-files" nicht möglich */
	/* standardfall: SearchPath auf ./ setzen */
	else if(strchr(options[1], '-') != NULL)
	{
		strcpy(result, "./");
	}
	else
	{
		strcpy(result, options[1]);
	}
	
	/* FB Gruppe7: return bei void? */
	return;
}

/**
 * \brief Die Funktion findWithArguments validiert die Suchparameter.
 *
 * Zuerst erfolgt Pruefung ob ein Parameter uebergeben wurde, anschließend werden alle uebergebene Parameter mit den validen Suchparameter verglichen.
 * 
 *
 * \param *dirp
 * \param *argv
 * \param *path
 * \param inputfileStat
 */
void findWithArguments(struct dirent *dirp, char *argv[], char *path, struct stat inputfileStat)
{
	/**
	 * \brief Parameterbeschreibung
	 * 
	 * \param bool printed ist default auf false gesetzt. Wird gesetzt sobald ein '-ls' oder ein '-print' uebergeben wurde, 
	 * mit welchem die gefundenen Parameter bereits ausgedruckt wurden.
	 * Sollte 'printed' auf false bleiben (weil kein '-print' oder '-ls' uebergaeben wurde) wird standardmaeßig mit '-print' 
	 * das gefundene ausgegeben.
	 * 
	 * \param int argumentCount ist ein interner Zaehler. Wenn in der naechste uebergebene Parameter fuer die Ueberpruefung ausgewaehlt 
	 * wird wird 'argumentCount hochgezaehlt. Solange bis es keine Parameter mehr zum ueberpruefen gibt.
	 * 
	 * \param int optionsCount ist ein interner Zaehler, der automatisch hochgezaehlt wird wenn eine Suchoption ('-name', '-type', ect.) 
	 * mit einem weiteren Parameter ueberprueft wird. Gibt es einen Match mit der Aufgabe wird 'foundCount' mit hochgezaehlt.
	 * Die beiden Parameter werden fuer die Print-Ausgabe geprueft. Sollten sie nicht ubereinstimmen wird nicht gedruckt, da die 
	 * Suche nichts mit den Suchparameter uebereinstimmendes gefunden hat.
	 * 
	 * \param int foundCount ist ein interner Zaehler, der automatisch hochgezaehlt wird, wenn die Suchoptionen auf das File oder die Datei
	 * zutreffen.
	 * 
	 * \param struct passwd pwd bekommt aus den Funktionen 'getuserperid' und 'getuser' ein Struct mit den gefundenen Eintrag zurueck.
	 * Diese Werte werden hier benoetigt um gegen den Parameter fuer '-user' zu vergleichen. Es kann den UserID oder den UserName 
	 * als Parameter beinhalten. Die Benutzerzugehoerigkeits ID des Files oder des Ordners wird dann mit dem Ergebnis aus dem 
	 * Struct verglichen.
	 * 
	 * \param struct group grp1 bekommt aus der Funktion 'getgroup' ein Struct mit den gefundenen Gruppeneintrag zurueck.
	 * 
	 * \param int uid bekokmmt fuer die Ueberpruefung die IDs von den structs grp1 und pwd.
	 * 
	 * \param char name[200]
	 * 
	 * \param int magic die Variable magic hat eine ganz spezielle Aufgabe: casts umgehen. Uberall wo sonst ein Cast haette
	 * verwendet werden muessen wird es wie durch 'magic' erfolgreich umgewandelt. 
	 */
	bool printed = false;
	int argumentCount = 2;
	int optionsCount = 0;
	int foundCount = 0;
	struct passwd pwd;
	struct group grp1;
	int uid = 0;
	char name[200];
	/* FB Gruppe7: "magic" verhindert keine Typecasts sondern macht aus expliziten Typecasts implizite Typecasts
	   in den meisten Fällen hätte man die auszugebenden Werte mittels Formatstring %lu ausgeben können und so
	   Typecasts (implizit oder explizit) vermeiden können */
	int magic = 0;

	
	/* Wenn params.ParamterCount == 1 also keine Prametr uebergeben wurden --> drucke alles*/
	if (params.ParameterCount == 1)
	{
		printf("%s\n", path);
	}
	/* Wenn params.ParameterCount > 1 also Parameter wurden uebergeben --> pruefe durch*/
	else
	{
		while (argumentCount < params.ParameterCount)
		{
				/* FB Gruppe7: Einrückung erschwert Lesbarkeit */
				if (strcmp(argv[argumentCount],PARAM_NAME) == 0)
			{
				optionsCount++;
				/* dirp kann NULL sein. Wenn zb. als Uebergabeparameter in argv[1] ein Pfadname + Dateiname steht, dann gibt es kein
				 * Directory weil hier dann das uebergebene File (in argv[1]) direkt geprueft wird. 
				 */
				if(dirp != NULL)
				{
					if (fnmatch(argv[argumentCount+1], dirp->d_name, FNM_PATHNAME) == 0)
					{
						foundCount++;
					}
				}
				else
				{
					if (fnmatch(argv[argumentCount+1], basename(path), FNM_PATHNAME) == 0)
					{
						foundCount++;
					}
					
				}

				/* FB Gruppe7: Sich wiederholende Abbruchbedingungen, hätte durch zusätzliche Schleife vermieden werden können */
				if (++argumentCount < params.ParameterCount)
				{
					continue;
				}
				else
				{
					break;
				}
			}

			if (strcmp(argv[argumentCount], PARAM_USER) == 0)
			{
				
				pwd = getuser(argv[argumentCount + 1]);
				optionsCount++;
				if (inputfileStat.st_uid == pwd.pw_uid)
				{
					
					foundCount++;
					/* FB Gruppe7: Sich wiederholende Abbruchbedingungen, hätte durch zusätzliche Schleife vermieden werden können */
					if (++argumentCount < params.ParameterCount)
					{
						continue;
					}
					else
					{
						break;
					}
				}
				
			}
			if (strcmp(argv[argumentCount], "-type") == 0)
			{
				optionsCount++;
				if(((S_ISDIR(inputfileStat.st_mode) != 0) && (strcmp(argv[argumentCount + 1], "d") == 0))
						|| ((S_ISCHR(inputfileStat.st_mode) != 0) && (strcmp(argv[argumentCount + 1], "c") == 0))
						|| ((S_ISBLK(inputfileStat.st_mode) != 0) && (strcmp(argv[argumentCount + 1], "b") == 0))
						|| ((S_ISFIFO(inputfileStat.st_mode) != 0) && (strcmp(argv[argumentCount + 1], "p") == 0))
						|| ((S_ISREG(inputfileStat.st_mode) != 0) && (strcmp(argv[argumentCount + 1], "f") == 0))
						|| ((S_ISLNK(inputfileStat.st_mode) != 0) && (strcmp(argv[argumentCount + 1], "l") == 0))
						|| ((S_ISSOCK(inputfileStat.st_mode) != 0) && (strcmp(argv[argumentCount + 1], "s") == 0)))
				{
					foundCount++;
					
				}
				/* FB Gruppe7: Sich wiederholende Abbruchbedingungen, hätte durch zusätzliche Schleife vermieden werden können */
				if (++argumentCount < params.ParameterCount)
				{
					continue;
				}
				else
				{
					break;
				}
			}

			if (strcmp(argv[argumentCount], PARAM_PRINT) == 0)
			{
				if (optionsCount == foundCount)
				{
					print(path);
				}

				printed = true;
				/* FB Gruppe7: Sich wiederholende Abbruchbedingungen, hätte durch zusätzliche Schleife vermieden werden können */
				if (++argumentCount < params.ParameterCount)
				{
					continue;
				}
				else
				{
					break;
				}
			}

			if (strcmp(argv[argumentCount], PARAM_NOUSER) == 0)
			{
				uid = inputfileStat.st_uid;
				sprintf(name, "%d", uid);
				pwd = getuserperid(name);
				optionsCount++;
				magic = pwd.pw_uid;
				if (uid != magic)
				{
					foundCount++;
				}
				/* FB Gruppe7: Sich wiederholende Abbruchbedingungen, hätte durch zusätzliche Schleife vermieden werden können */
				if (++argumentCount < params.ParameterCount)
				{
					continue;
				}
				else
				{
					break;
				}
			}

			if (strcmp(argv[argumentCount], PARAM_PATH)== 0)
			{
				optionsCount++;
				if(fnmatch(argv[argumentCount + 1], path, FNM_NOESCAPE) == 0)
				{
					foundCount++;
				}
				/* FB Gruppe7: Sich wiederholende Abbruchbedingungen, hätte durch zusätzliche Schleife vermieden werden können */
				if (++argumentCount < params.ParameterCount)
				{
					continue;
				}
				else
				{
					break;
				}
			}

			if (strcmp(argv[argumentCount], PARAM_LS) == 0)
			{
				/* print gefundenen Eintrag mit ls*/
				if (optionsCount == foundCount)
				{
					/* ino = inputfileStat.st_ino;*/
					printf("%6lu\t", inputfileStat.st_ino);
					printf("%4lu ", (inputfileStat.st_blocks /2 ));
					
					getprotection(inputfileStat);
					
					magic = inputfileStat.st_nlink;
					printf("\t%d", magic);
					/*printf("\t%ld", inputfileStat.st_nlink);*/
					
					uid = inputfileStat.st_uid;
					sprintf(name, "%d", uid);
					pwd = getuserperid(name);
					magic = pwd.pw_uid;
					/* FB Gruppe7: uid ist nur dann ungleich magic wenn pwd == NULL sprich kein User gefunden - da hätte man gleich auf pwd == NULL checken können,
					   hätte den Code deutlich verständlicher gemacht */
					if (uid != magic)
					{
						printf(" %d\t", uid);
					}
					else
					{
						printf(" %s\t", pwd.pw_name);
					}
					uid = inputfileStat.st_gid;
					sprintf(name, "%d", uid);
					grp1 = getgroup(name);
					magic = grp1.gr_gid;
					
					/* FB Gruppe7: uid ist nur dann ungleich magic wenn pwd == NULL sprich kein User gefunden - da hätte man gleich auf pwd == NULL checken können,
					   hätte den Code deutlich verständlicher gemacht */
					if (uid != magic)
					{
						printf("%d\t", uid);
					}
					else
					{
						printf("%s\t", grp1.gr_name);
					}
					printf("%10ld ", inputfileStat.st_size);
					gettime(inputfileStat);
					print(path);
				}
				printed = true;
				if (++argumentCount < params.ParameterCount)
				{
					continue;
				}
				else
				{
					break;
				}
			}
			argumentCount++;
		}/*while ende*/

		if (printed == false && optionsCount == foundCount)
		{
			print(path);
		}

	}/*else if ende*/
}
/**
 * \brief Die Funktion print dient zur Ausgabe des Pfades
 *
 * Es wird lediglich der Pfad übergeben und mittels printf ausgegeben
 *  *
 * \param *path
 */
void print(char *path)
{
	/* FB Gruppe7: gute Idee das in eine eigene Funktion auszulagern hätte man eine Fehlerbehandlung gemacht, falls printf() (auf stdout) fehlschlägt */
	printf("%s\n", path);
}
/**
 * \brief passwd getuser 
 *
 *
 * 
 * \param *username
 * \retval user
 */
struct passwd getuser(char *username)
{
	struct passwd user;
	/* FB Gruppe7: Geschmackssache, aber wir hätten dafür keine eigene Pointervariable deklariert und "user" mit Adressoperator angesprochen */
	struct passwd *pwdptr = &user;
	struct passwd *result;
	char pwdbuffer[200];
	int pwdlenght = sizeof(pwdbuffer);
	char *pend;
	long uid = 0;

	
		getpwnam_r(username, pwdptr, pwdbuffer, pwdlenght, &result);
	
		if(result == NULL)
		{
		/* FB Gruppe7: kein Check ob "username" nur numerisch ist, "uid" bekommt nur numerische Werte bis zum 1. Alphanumerischen Wert in "username" zugewiesen.
		   siehe auch Anmerkung in Funktion checkIfUserExists() */
		uid = strtol(username, &pend, 10);
		getpwuid_r(uid, pwdptr, pwdbuffer, pwdlenght, &result);
		}

	return user;
}
/**
 * \brief passwd getuserpername 
 *
 * 
 *  
 * \param *username
 * \retval user
 */

/* FB Gruppe7: Funktion wird nie aufgerufen */

struct passwd getuserpername(char *username)
{
	struct passwd user;
	struct passwd *pwdptr = &user;
	struct passwd *result;
	char pwdbuffer[200];
	int pwdlenght = sizeof(pwdbuffer);
	

	getpwnam_r(username, pwdptr, pwdbuffer, pwdlenght, &result);

	return user;
}
/**
 * \brief passwd getuser perid
 *
 * 
 *  
 * \param *username
 * \retval user
 */
struct passwd getuserperid(char *username)
{
	struct passwd user;
	/* FB Gruppe7: Geschmackssache, aber wir hätten dafür keine eigene Pointervariable deklariert und "user" mit Adressoperator angesprochen */
	struct passwd *pwdptr = &user;
	struct passwd *result;
	char pwdbuffer[200];
	int pwdlenght = sizeof(pwdbuffer);
	char *pend;
	long uid = 0;
	/* FB Gruppe7: kein Check ob "username" nur numerisch ist, "uid" bekommt nur numerische Werte bis zum 1. Alphanumerischen Wert in "username" zugewiesen.
	   siehe auch Anmerkung in Funktion checkIfUserExists() */
	uid = strtol(username, &pend, 10);
	getpwuid_r(uid, pwdptr, pwdbuffer, pwdlenght, &result);

	return user;
}
/**
 * \brief group getgroup
 *
 * 
 * 
 *
 * \param* groupname
 * \return true oder false
 */
struct group getgroup(char *groupname)
{
	struct group grp;
	/* FB Gruppe7: Geschmackssache, aber wir hätten dafür keine eigene Pointervariable deklariert und "grp" mit Adressoperator angesprochen */
	struct group *grpptr = &grp;
	struct group *result;
	char grpbuffer[200];
	int grplenght = sizeof(grpbuffer);
	char *pend;
	long uid = 0;

	/* FB Gruppe7: kein Check ob "username" nur numerisch ist, "uid" bekommt nur numerische Werte bis zum 1. Alphanumerischen Wert in "username" zugewiesen.
	   siehe auch Anmerkung in Funktion checkIfUserExists() */
	uid = strtol(groupname, &pend, 10);
	
	
	getgrgid_r(uid, grpptr, grpbuffer, grplenght, &result);
	

	return grp;
}
/**
 * \brief Die Funktion getprotection dient der Feststellung  welche Berechtigungen ein FILE / Directory usw. hat und Ausgabe dieser
 *
 *
 * \param inputfileStat
 */
void getprotection(struct stat inputfileStat)
{
	if(S_ISDIR(inputfileStat.st_mode))
	{
		printf("d");
	}
	else if(S_ISCHR(inputfileStat.st_mode))
	{
		printf("c");
	}
	else if(S_ISBLK(inputfileStat.st_mode))
	{
		printf("b");
	}
	
	else if(S_ISFIFO(inputfileStat.st_mode))
	{
		printf("p");
	}
	
	else if(S_ISLNK(inputfileStat.st_mode))
	{
		printf("l");
	}
	else if(S_ISSOCK(inputfileStat.st_mode))
		{
			printf("s");
		}
	else
	{
		printf("-");
	}
	
	printf((inputfileStat.st_mode & S_IRUSR) ? "r" : "-");
	printf((inputfileStat.st_mode & S_IWUSR) ? "w" : "-");
	if((inputfileStat.st_mode & S_ISUID))
	{
		printf((inputfileStat.st_mode & S_IXUSR) ? "s" : "S");
	}
	else if((inputfileStat.st_mode & S_IXUSR))
	{
	printf((inputfileStat.st_mode & S_IXUSR) ? "x" : "-");
	}
	else
	{
		printf("-");
	}
	printf((inputfileStat.st_mode & S_IRGRP)? "r" : "-");
	printf((inputfileStat.st_mode & S_IWGRP)? "w" : "-");
	if((inputfileStat.st_mode & S_ISGID))
	{
		printf((inputfileStat.st_mode & S_IXGRP)? "s" : "S");
	}
	else if((inputfileStat.st_mode & S_IXGRP))
	{
		printf((inputfileStat.st_mode & S_IXGRP)? "x" : "-");
	}
	else
		{
			printf("-");
		}
	printf((inputfileStat.st_mode & S_IROTH)? "r" : "-");
	printf((inputfileStat.st_mode & S_IWOTH)? "w" : "-");
	if((inputfileStat.st_mode & S_ISVTX))
		{
			printf((inputfileStat.st_mode & S_IXOTH)? "t" : "T");
		}
	else if((inputfileStat.st_mode & S_IXOTH))
	{
		printf((inputfileStat.st_mode & S_IXOTH)? "x" : "-");
	}
	else
		{
			printf("-");
		}
}
/**
 * \brief Die Funktion gettime dient dazu die Zeit des letzten Zugriffs auf das File festzustellen und auszugeben.
 *
 * \param inputfileStat
 */
void gettime(struct stat inputfileStat)
{
	struct tm *tm;
	/* FB Gruppe7: recht weit... */
	char datastring[256];
	char *time_fmt = NULL;
	
	if (time_fmt == NULL )
	{
		/* FB Gruppe7: %e ist eine SU extension */
		time_fmt = "%b  %e %R";
	}
	tm = localtime(&inputfileStat.st_mtime);
	
	strftime(datastring, sizeof(datastring), time_fmt, tm);
	printf("%s ", datastring);

}
/**
 * \brief Die Funktion checkIfUserExist prueft ob ein uebergebener Username existiert
 *
 * 
 * 
 *
 * \param *username
 * \retval found
 */
bool checkIfUserExists(char *username)
{
	struct passwd *pw;
	char *pend;
	uint id = 0;
	bool found = false;

	setpwent();
	while ((pw = getpwent()) != NULL )
	{
		/* FB Gruppe7: ein Username kann durchaus aus teilweise oder auch komplett numerischen Teilen bestehen, dieses Vorgehen bewirkt falsche Ergebnisse
		   Bsp: UID 0; Beginn mit bekannter UID und wahllosen Zeichen etc. etc.
		   Weiters: id ist als unsigned int definiert, strol liefert aber einen signed long zurück - dennoch keine Compilererrors?! */ 
		id = strtol(username, &pend, 10);
		if (id == 0)
		{
			if (strcmp(pw->pw_name, username) == 0)
			{
				found = true;
			}
		}
		else
		{
			if (pw->pw_uid == id)
			{
				found = true;
			}
		}
	}

	/* FB Gruppe7: Aufruf von endpwent() fehlt */

	if (id != 0 && !found)
	{
		exit(EXIT_FAILURE);
	}

	return found;
}
/**
 * \brief Die Funktion typeValidation dient der Feststellung ob einer der Parameter 'bcdpfls' übergeben wurde.
 *
 * Trifft es zu, dass einer der Parameter übergeben wurde so wird true an die Variable Result übergeben welche wiederum Rückgabewert der Funktion ist
 *
 *
 * \param *searchOptions
 * \retval result
 */
bool typeValidation (char *searchOptions)
{
	bool result = false;

	if((searchOptions[0] == 'b')
		||(searchOptions[0] == 'c') 
		||(searchOptions[0] == 'd')
		||(searchOptions[0] == 'p')
		||(searchOptions[0] == 'f')
		||(searchOptions[0] == 'l')
		||(searchOptions[0] == 's'))
	{
		result = true;
	}
	/* FB Gruppe7: else-Zweig überflüssig da result bereits mit "false" initialisiert */
	else
	{
		result = false;
	}

	return result;
}
/*
 * =================================================================== eof ==
 */
