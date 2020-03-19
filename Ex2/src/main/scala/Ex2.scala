import org.apache.spark.SparkContext
import org.apache.spark.SparkConf

object MyFunctions {
    def addAAtLast(s: String): String = { s + "A" }
}

object Ex2 {
  def main(args: Array[String]) {
    val conf = new SparkConf().setAppName("Ex2")
    val sc = new SparkContext(conf)
    val lines = sc.textFile("data.txt", 4)

    // If map is used, collection Array[String] is returned
    // flatMap() -> Converts each element into an RDD
    // filter() -> used here to remove empty lines
    val words = lines.flatMap(l => l.split(" ")).filter(l => l != "")
    val wordTuples = words.map(l => (l, 1))
    val countsOfWords = wordTuples.reduceByKey((a, b) => (a + b))
    countsOfWords.collect().foreach(println)
  }
}
