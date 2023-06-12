var ethers = require("ethers");
const mongoose = require('mongoose');
const PriceFeed = require('./models/priceFeedModel');
const abi = require('./abi.json');
// .doenv
const dotenv = require('dotenv');
dotenv.config();

var url = process.env.SEPOOL_URL;
var provider = new ethers.providers.JsonRpcProvider(url);
provider.getBlockNumber().then((result) => {
  console.log("Current block number: " + result);
});


const contractAddress = '0x88f749EE2a9F6bb4a0BBBbC83493FfE7ac960831';
const contract = new ethers.Contract(contractAddress, abi, provider);


async function fetchPrice() {
    try {
      const latestData = await contract.getLatestData();
      console.log('Latest Data:', latestData.toString());
    } catch (error) {
      console.error('Error:', error);
    }
  }

const pushPrice = async () => {
  try {
    const latestData = await contract.getLatestData();
    const price = latestData.toString();
    const timestamp = Date.now();
    const priceFeed = new PriceFeed({ price, timestamp });
    await priceFeed.save();
    console.log('Price feed saved:', priceFeed);
  }
  catch (error) {
    console.error('Error:', error);
  
  }
}
  mongoose
  .connect(
    "mongodb+srv://reflowchain:reflowchain@reflowchain.r4hxsjs.mongodb.net/products?retryWrites=true&w=majority"
  )
  .then(() => {
    console.log("Connected to database!");
  })
  .catch(() => {
    console.log("Connection failed!");
  });
pushPrice();
  
