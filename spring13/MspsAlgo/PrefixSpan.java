import java.awt.List;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.LineNumberReader;
import java.io.PrintStream;
import java.io.PrintWriter;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Scanner;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class PrefixSpan {
	
	double lines = 0;
	ArrayList<Double> misv = new ArrayList<Double>();
	ArrayList<Integer> num = new ArrayList<Integer>();
	double SDC;
	int mcount[] = new int[100];	//stores count of index
	ArrayList<SeqStruct> ss = new ArrayList<SeqStruct>();
	OutputData oDB = new OutputData();
	
	void findLinesinData() throws Exception
	{
		LineNumberReader  lnr = new LineNumberReader(new FileReader(new File("data.txt")));
		lnr.skip(Long.MAX_VALUE);
		lines = lnr.getLineNumber()+1;
		
	}
		
	public static void main(String[] args) throws Exception {
		PrefixSpan ps = new PrefixSpan();
		ps.findLinesinData();
		ps.read_files();
		ps.findLinesinData();
		ps.checkForMinSup();
		ps.generateSubs();
		ps.printOutput();
		String temp = new String();
		temp = " ";
		
	}
	
	void read_files() throws Exception
	{
		// Read data File
		File file = new File("data.txt");
        Scanner scanner = new Scanner(file);
        
        while (scanner.hasNextLine()) // reads the input line by line....and performs the specified operation as specified in transfile.
        {
        	
        	String line = scanner.nextLine();
        	SeqStruct sst = new SeqStruct();
        	ArrayList<String> matchList = new ArrayList<String>();
            
            //extracts individual seq from a trans
            
            Pattern regex = Pattern.compile("\\{([^}]*)\\}");
            Matcher regexMatcher = regex.matcher(line);
            while (regexMatcher.find()) 
            {
            	matchList.add(regexMatcher.group());
            	sst.s.add(matchList.get(matchList.size()-1));
            	
            	String temp = matchList.get(matchList.size()-1);
            	
            	int k = 0;
            	Pattern p = Pattern.compile("\\d+");
                Matcher m = p.matcher(temp); 
                while (m.find()) 
                   k++;
                sst.counts.add(k);
            }
            
            // extracts all numbers from input string
            Pattern p = Pattern.compile("\\d+");
            Matcher m = p.matcher(line); 
            ArrayList<Integer> tmp = new ArrayList<Integer>();
            while (m.find())
            {
               sst.arrayOfNumbers.add(Integer.parseInt(m.group()));
               tmp.add(Integer.parseInt(m.group()));
               if(tmp.lastIndexOf(Integer.parseInt(m.group())) == tmp.indexOf(Integer.parseInt(m.group())))
            	   mcount[(Integer.parseInt(m.group()))] += 1;
            }
            tmp.clear();
            for( int i = 0; i<sst.s.size();i++ )
            {
            	for(int j = 0; j < (int)sst.counts.get(i)-1 ;j++)
            	{
            		sst.form.add(true);
            	}
            	sst.form.add(false);
            }
            
            	ss.add(sst);
        }
        
        scanner.close(); 
        
        // Read MIS File
        File misfile = new File("para.txt");
        Scanner misscanner = new Scanner(misfile);
        int i =1;
        while (misscanner.hasNextLine()) // reads the input line by line....and performs the specified operation as specified in transfile.
        {
        	String line = misscanner.nextLine();
            Scanner s = new Scanner(line);
        	if(s.next().equalsIgnoreCase("SDC") )
        	{
        		s.next();
        		SDC = s.nextDouble();
        	}
        	else{
        		
        		num.add(i);
        		        		
        		String[] srt = line.split(" ");
        		misv.add(Double.parseDouble(srt[2]));
        		
      	}
        	
        	
        	
        i++;
        s.close();
        }
        
        misscanner.close();
		
	}
	
	void checkForMinSup()
	{
		for(int i = 0; i< num.size();i++)
		{
			if (i+1 ==(int) num.get(i) && (mcount[i+1] / lines ) < misv.get(i))
			{
				misv.set(i, (double) 10000);
				num.set(i,10000);
			}
		}
		for(int i = num.size()-1; i>=0;i--)
		{
			if(misv.get(i)== (double)10000)
			{
				misv.remove(i);
				num.remove(i);
			}
		}
	//SORT ACCORDING TO MIS
	}
	
	int findSupport(ArrayList<String> s, int a)
	{
		int count = 0;
		for(int i = 0; i< s.size();i++)
		{
			String line = s.get(i);
			Pattern p = Pattern.compile("\\d+");
			Matcher m = p.matcher(line); 
			while (m.find())
			{
				if(a == Integer.parseInt(m.group()))
				{
					count++;break;
				}
				
			}
		}
		if(count>0)
			return count;
		else
			return -1;
	}
	
	void generateSubs()
	{
		
		for(int i = 0; i<num.size();i++)
		{
			ArrayList<String> sub = new ArrayList<String>();
			ArrayList<SeqStruct> ssub = new ArrayList<SeqStruct>();
			
			for(int j = 0;j < ss.size(); j++)	// ss is input database
			{
				SeqStruct stemp = new SeqStruct();
				if(ss.get(j).arrayOfNumbers.contains((int)num.get(i))) //if seq contains the current no then generate sequence
				{
					ArrayList<Integer> nums = new ArrayList<Integer>();
					
					for(int l = 0; l< ss.get(j).arrayOfNumbers.size();l++ )
					{
						double  n1 = mcount[(int) num.get(i)]/lines;
						double n2 = mcount[(int) ss.get(j).arrayOfNumbers.get(l)]/lines;
						if(Math.abs( n1 - n2) <= SDC)
						{
							//if minsupp condition holds generate sequence
							
							nums.add((int) ss.get(j).arrayOfNumbers.get(l));
						}
						else
							nums.add(0);
					}
					int it = 0;
					for(int l = 0; l < ss.get(j).counts.size(); l++)
					{
						String tempseq= new String();
						tempseq += "{";
						int t = it;
						int ct = 0;
						for(int p = it; p < t+ ss.get(j).counts.get(l);p++)
						{
							
							if(nums.get(p) != 0)
							{
								tempseq += nums.get(p).toString() + ", ";
								stemp.arrayOfNumbers.add(nums.get(p));
								stemp.form.add(true);
								ct++;
							}
							it++;
						}
						if(ct != 0)
							stemp.counts.add(ct);
						if(stemp.form.size()!=0)
						stemp.form.set(stemp.form.size()-1, false);
						if (tempseq.endsWith(", ")) 
						{
							tempseq = tempseq.substring(0, tempseq.length() - 2);
							tempseq += "}";
						}
						else if(tempseq.endsWith("{"))
						{
							tempseq = tempseq.substring(0, tempseq.length() - 1);
						}
						if(!tempseq.equals("") &&  !tempseq.equals(" "))
							stemp.s.add(tempseq);						
					}
					
					//now consider form and numbers
					ssub.add(stemp);
				}//end if
				
				}
			ArrayList<String> seqs = new ArrayList<String>();
			for( int z = 0; z<ssub.size();z++)
			{
				String tmp = new String();
				for(int j = 0; j<ssub.get(z).s.size();j++)
				{
					tmp += ssub.get(z).s.get(j);
				}
					seqs.add(tmp);
			}
			prefixSpan(ssub,seqs,num.get(i).toString(),((int)Math.ceil(misv.get(i) * lines)));
		}
	}
	
	void prefixSpan(ArrayList<SeqStruct> S,ArrayList<String> seqs, String ik, int min_Sup )
	{
		
		if(ik.equals("3"))
		{
			int d = 0;
			d++;
		}
		
		ArrayList<SeqStruct> para1 = S;
		int para2 = min_Sup;// To be used for recursive calls
		ArrayList<String> para3 = seqs;
		ArrayList<Integer> tmp = new ArrayList<Integer>(); //contains all numbers in ik
		String ikchop = new String(ik); 
		Scanner scnr = new Scanner(ikchop);
		if(scnr.hasNextInt())
		{
			//if string is one number then add to output....
			oDB.os.add("{"+ik+"}");
			oDB.oc.add(mcount[scnr.nextInt()]);
			oDB.length.add(1);
			scnr.close();
			
			tmp.add(Integer.parseInt(ik));
			
		}
		else
		{
			// extract the input string..
			Pattern p = Pattern.compile("\\d+");
            Matcher m = p.matcher(ik); 
            while (m.find())
            {
            	tmp.add(Integer.parseInt(m.group()));
            }
		}
		
		
		for(int y = 0; y < S.size();y++)
		{
			for(int x = 0; x < S.get(y).arrayOfNumbers.size(); x++)
			{
				if(findSupport(para3,S.get(y).arrayOfNumbers.get(x)) < para2 )
					S.get(y).arrayOfNumbers.set(x, -1);
			}
			
		}
		
		//find form of ik
		ArrayList<Boolean> seqsForm = new ArrayList<Boolean>();
		ArrayList<String> matchList = new ArrayList<String>();
        
        //extracts individual seq from a trans
        
        Pattern regex = Pattern.compile("\\{([^}]*)\\}");
        Matcher regexMatcher = regex.matcher(ik+"}");
        while (regexMatcher.find()) 
        {
        	matchList.add(regexMatcher.group());
        	
        	
        	String temp = matchList.get(matchList.size()-1);
        	Scanner scn = new Scanner(temp.substring(1, temp.length()-1));
        	scn.useDelimiter(",");
        	while(scn.hasNextInt())
        	{
        		scn.nextInt();
        		seqsForm.add(true);
        	}
        	seqsForm.set(seqsForm.size()-1, false);
        
        }
       
        Scanner scnr1 = new Scanner(ik);
		if(scnr1.hasNextInt())
		{
			seqsForm.add(false);
		}
		scnr1.close();
		
		//find if all ik is present and generate pairs.. THATS IT!
		
		for(int i = 0; i<seqs.size();i++)//for every sequence
		{
			
		//	if(S.get(i).arrayOfNumbers.size() == 1 || S.get(i).arrayOfNumbers.size() == 0 )
		//		break;
			int postostart = 0;
			boolean allnumpresent = true;
			int rem = 0;
			String ikcall;
			
			//for every element in ik
			for(int j = 0; j< tmp.size(); j++)
			{
				for(int l = postostart; l < S.get(i).arrayOfNumbers.size(); l++) //if this loop exits without executing break then prefix is present else move to next transaction.
				{
					if(tmp.get(j) == S.get(i).arrayOfNumbers.get(l))
					{
						postostart = l+1;	// so that when all prefix is found we start pair from next pos of where last prefix element was present
						allnumpresent = true;
						break;
					}
					else
						allnumpresent = false;
				}
			}
				
		/*	boolean dflag = true;
			for(int j = S.get(i).arrayOfNumbers.indexOf(tmp.size()-2); j < S.get(i).arrayOfNumbers.size() && j >= 0;j++)
			{
				if(S.get(i).form.get(j))
				{}
				else
				{
					dflag = false;
					break;
				}
			}
			*/
			
			for( int j = postostart + 1; j< S.get(i).arrayOfNumbers.size();j++)
			{
				if(tmp.get(tmp.size()-1) == S.get(i).arrayOfNumbers.get(j) && seqsForm.size()>=2)
				{
					if(!seqsForm.get(tmp.size()-2)/* dflag */&& tmp.get(tmp.size()-1) != tmp.get(tmp.size()-2))
					{
						postostart = j+1;	// so that when all prefix is found we start pair from next pos of where last prefix element was present
						allnumpresent = true;
					}
				}
			}
		
			// check supp of S.get(i).arrayOfNumbers.get(k)
			
			
			if(allnumpresent)
			{

				//find supp of next element pair or not?
			//if(postostart < S.get(i).arrayOfNumbers.size())	
			//{
		
			for(int k = postostart; k < S.get(i).arrayOfNumbers.size() ;k++)
			{
					
				
				if(S.get(i).arrayOfNumbers.get(k) != -1 && tmp.get(0) != S.get(i).arrayOfNumbers.get(k) )
				{
					// dont form pair
			//	}
		//	else
		//		{
				// form pair
				
						//int a = postostart;
						boolean formflag = S.get(i).form.get(postostart-1);
							//if all elements between startpos and element being looked at are in same bracket then next element also goes into same bracket.
					if(formflag)
						for(int a = postostart ; a < k; k++)
						{
							if( S.get(i).form.get(a))
							{/*do nothing, flagform remains true */}
							else
							{
									formflag=false;
									break;
							}
						}

					//ADJUST BRACKETS
					if(S.get(i).form.get(k-1) && formflag)
					{
						if(ik.length()>=1 &&  ik.charAt((ik.length()-1)) == '}')
							ik = ik.substring(0, ik.length()-1);
						ikcall = ik +","+ (S.get(i).arrayOfNumbers.get(k)).toString();
					}
					else
					{
						if(ik.charAt((ik.length()-1)) != '}')
							ik = ik + "}";
						
						ikcall = ik +"{"+ (S.get(i).arrayOfNumbers.get(k)).toString();
					}

					if(!ikcall.startsWith("{"))
						ikcall = "{" + ikcall;
					
					int repeatornot = 0;
					
					if(!oDB.os.contains(ikcall+"}") )
					{
						oDB.os.add(ikcall+"}");
						oDB.oc.add(1);
						oDB.length.add(tmp.size()+1);
						repeatornot = i;
						if(!ik.equals(ikcall))
							prefixSpan(para1,para3,ikcall,para2);
					}
					else
					{
						//if(repeatornot != i &&  repeatornot>0)
					//	{
						int cindex = oDB.os.indexOf(ikcall+"}");
						int getcount = oDB.oc.get(cindex);
						getcount++;
						oDB.oc.set(cindex,getcount);
					//	}
					}
			}
				}
	/*	}
			else
			{
				if(oDB.os.contains(ik+"}") )
				{
					int cindex = oDB.os.indexOf(ik+"}");
					int getcount = oDB.oc.get(cindex);
					getcount++;
					oDB.oc.set(cindex,getcount);
				}
				
			}*/
			

			}
		}
	
		}
	
	void printOutput() throws IOException
	{
		
		
		//PrintWriter out = new PrintWriter(new FileWriter("output.txt"));
		PrintStream out = new PrintStream(new FileOutputStream("output.txt"));
		System.setOut(out);
		
		//find max length
		int max = 0;
		for(int i = 0; i< oDB.length.size(); i++)
		{
			if(oDB.length.get(i) > max)
				max = oDB.length.get(i);
		}
		
		for(int i = 1; i <= max; i++)
		{
			int countOfLengthPatterns = 0;
			for(int j= 0; j<oDB.os.size(); j++)
				if(oDB.length.get(j)==i)
					countOfLengthPatterns++;	
			
			
			System.out.println("The number of length "+i+" sequential patterns are -->"+ countOfLengthPatterns +" listed below: ");
		//	out.println("The number of length "+i+" sequential patterns are -->"+ countOfLengthPatterns +" listed below: ");		
			for(int j= 0; j<oDB.os.size(); j++)
				if(oDB.length.get(j)==i)
				{
					System.out.println("Pattern: " + oDB.os.get(j) + "		Count: " + oDB.oc.get(j));
			//		out.println("Pattern: " + oDB.os.get(j) + "		Count: " + oDB.oc.get(j));
				}
		}
		
		
		
	}
		
		
	
}
