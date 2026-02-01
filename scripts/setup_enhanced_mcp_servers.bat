@echo off
echo Setting up Enhanced MCP Servers for Autonomous AI Development
echo ===============================================================

cd mcp-servers-source

echo Installing dependencies for all servers...

echo Installing database server dependencies...
cd src/database
npm install
cd ../..

echo Installing container server dependencies...
cd src/container
npm install
cd ../..

echo Installing testing server dependencies...
cd src/testing
npm install
cd ../..

echo Building all servers...
cd src/database
npm run build
cd ../..

cd src/container
npm run build
cd ../..

cd src/testing
npm run build
cd ../..

cd ../..

echo Creating MCP configuration...
echo Project MCP config: .cursor\mcp.json
echo Standalone config: enhanced_mcp_config.json
echo.

echo Setup complete! Enhanced MCP servers are ready for autonomous AI development.
echo.
echo Configuration file created: enhanced_mcp_config.json
echo.
echo Next steps:
echo 1. Copy the configuration from enhanced_mcp_config.json to your MCP client
echo 2. Restart your MCP client to load the new servers
echo 3. Test the enhanced capabilities with autonomous development tasks
echo.
pause