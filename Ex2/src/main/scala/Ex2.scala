import org.apache.spark.SparkContext
import org.apache.spark.SparkConf

object Ex2 {
  def main(args: Array[String]) {
    val conf = new SparkConf().setAppName("Ex2")
    val sc = new SparkContext(conf)

    var fileName = ""
    if(args.length == 1)
        fileName = args(0)
    else {
        println("Usage : spark-submit [<...>] <file name>")
        return
    }
    println("File Name : " + fileName)
    // Using 4 partitions
    //val lines = sc.textFile(fileName, 4)
    val lines = sc.textFile(fileName)

    // If map is used, collection Array[String] is returned
    // flatMap() -> Converts each element into an RDD
    // filter() -> used here to remove empty lines
    val words = lines.flatMap(l => l.split(" ")).filter(l => l != "")
    val wordTuples = words.map(l => (l, 1))
    val countsOfWords = wordTuples.reduceByKey((a, b) => (a + b))
    countsOfWords.collect().foreach(println)
  }
}
